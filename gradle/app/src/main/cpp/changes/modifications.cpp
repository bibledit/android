/*
 Copyright (©) 2003-2024 Teus Benschop.
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include <changes/modifications.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/usfm.h>
#include <filter/text.h>
#include <filter/diff.h>
#include <filter/shell.h>
#include <filter/passage.h>
#include <filter/date.h>
#include <html/text.h>
#include <text/text.h>
#include <database/logs.h>
#include <database/modifications.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/statistics.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <client/logic.h>
#include <access/bible.h>
#include <config/globals.h>
#include <changes/logic.h>
#include <styles/css.h>
#include <email/send.h>
using namespace std;


// Internal function declarations.
void changes_process_identifiers (Webserver_Request& webserver_request,
                                  const string & user,
                                  const vector <string> & recipients,
                                  const string & bible,
                                  int book, int chapter,
                                  const map <string, vector<string>> & bibles_per_user,
                                  int oldId, int newId,
                                  string & email,
                                  int & change_count, float & time_total, int & time_count);


// Helper function.
// $user: The user whose changes are being processed.
// $recipients: The users who opted for receiving online notifications of any contributor.
void changes_process_identifiers (Webserver_Request& webserver_request,
                                  const string & user,
                                  const vector <string> & recipients,
                                  const string & bible,
                                  int book, int chapter,
                                  const map <string, vector<string>> & bibles_per_user,
                                  int oldId, int newId,
                                  string & email,
                                  int & change_count, float & time_total, int & time_count)
{
  if (oldId != 0) {
    Database_Modifications database_modifications {};
    string stylesheet = Database_Config_Bible::getExportStylesheet (bible);
    Database_Modifications_Text old_chapter_text = database_modifications.getUserChapter (user, bible, book, chapter, oldId);
    string old_chapter_usfm = old_chapter_text.oldtext;
    Database_Modifications_Text new_chapter_text = database_modifications.getUserChapter (user, bible, book, chapter, newId);
    string new_chapter_usfm = new_chapter_text.newtext;
    vector <int> old_verse_numbers = filter::usfm::get_verse_numbers (old_chapter_usfm);
    vector <int> new_verse_numbers = filter::usfm::get_verse_numbers (new_chapter_usfm);
    vector <int> verses = old_verse_numbers;
    verses.insert (verses.end (), new_verse_numbers.begin (), new_verse_numbers.end ());
    verses = filter::strings::array_unique (verses);
    sort (verses.begin(), verses.end());
    for (auto verse : verses) {
      string old_verse_usfm = filter::usfm::get_verse_text (old_chapter_usfm, verse);
      string new_verse_usfm = filter::usfm::get_verse_text (new_chapter_usfm, verse);
      if (old_verse_usfm != new_verse_usfm) {
        Filter_Text filter_text_old = Filter_Text (bible);
        Filter_Text filter_text_new = Filter_Text (bible);
        filter_text_old.html_text_standard = new HtmlText (translate("Bible"));
        filter_text_new.html_text_standard = new HtmlText (translate("Bible"));
        filter_text_old.text_text = new Text_Text ();
        filter_text_new.text_text = new Text_Text ();
        filter_text_old.add_usfm_code (old_verse_usfm);
        filter_text_new.add_usfm_code (new_verse_usfm);
        filter_text_old.run (stylesheet);
        filter_text_new.run (stylesheet);
        string old_html = filter_text_old.html_text_standard->get_inner_html ();
        string new_html = filter_text_new.html_text_standard->get_inner_html ();
        string old_text = filter_text_old.text_text->get ();
        string new_text = filter_text_new.text_text->get ();
        if (old_text != new_text) {
          string modification = filter_diff_diff (old_text, new_text);
          email += "<div>";
          email += filter_passage_display (book, chapter, filter::strings::convert_to_string (verse));
          email += " ";
          email += modification;
          email += "</div>";
          if (webserver_request.database_config_user()->getUserUserChangesNotificationsOnline (user)) {
            database_modifications.recordNotification ({user}, changes_personal_category (), bible, book, chapter, verse, old_html, modification, new_html);
          }
          // Go over all the receipients to record the change for them.
          for (auto recipient : recipients) {
            // The author of this change does not get a notification for it.
            if (recipient == user) continue;
            // The recipient may have set which Bibles to get the change notifications for.
            // This is stored like this:
            // container [user] = list of bibles.
            bool receive {true};
            try {
              const vector <string> & bibles = bibles_per_user.at(recipient);
              receive = in_array(bible, bibles);
            } catch (...) {}
            if (!receive) continue;
            // Store the notification.
            database_modifications.recordNotification ({recipient}, user, bible, book, chapter, verse, old_html, modification, new_html);
          }
        }
        // Statistics: Count yet another change made by this hard-working user!
        change_count++;
        int timestamp = database_modifications.getUserTimestamp (user, bible, book, chapter, newId);
        time_total += static_cast<float>(timestamp);
        time_count++;
      }
    }
  }
}


void changes_modifications ()
{
  Database_Logs::log ("Change notifications: Generating", Filter_Roles::translator ());

  
  // Notifications are not available to clients to download while processing them.
  config_globals_change_notifications_available = false;
  
  
  // Data objects.
  Webserver_Request webserver_request {};
  Database_Modifications database_modifications {};


  // Check on the health of the modifications database and (re)create it if needed.
  if (!database_modifications.healthy ()) database_modifications.erase ();
  database_modifications.create ();
  
  
  // Get the users who will receive the changes entered by the named contributors.
  vector <string> recipients_named_contributors;
  {
    vector <string> users = webserver_request.database_users ()->get_users ();
    for (const auto & user : users) {
      if (webserver_request.database_config_user ()->getContributorChangesNotificationsOnline (user)) {
        recipients_named_contributors.push_back (user);
      }
    }
  }

  // Storage for the statistics.
  map <string, int> user_change_statistics {};
  float modification_time_total {0.0f};
  int modification_time_count {0};

  // There is a setting per user indicating which Bible(s) a user
  // will get the change notifications from.
  // This setting affects the changes made by all users.
  // Note: A user will always receive notificatons of changes made by that same user.
  map <string, vector<string> > notification_bibles_per_user {};
  {
    vector <string> users = webserver_request.database_users ()->get_users ();
    for (const auto & user : users) {
      vector <string> bibles = webserver_request.database_config_user ()->getChangeNotificationsBiblesForUser (user);
      notification_bibles_per_user [user] = bibles;
    }
  }
  
  
  // Create online change notifications for users who made changes in Bibles.
  // (The changes were made through the web editor or through a client).
  // It runs before the team changes.
  // This produces the desired order of the notifications in the GUI.
  // At the same time, produce change statistics per user.

  vector <string> users = database_modifications.getUserUsernames ();
  if (!users.empty ()) Database_Logs::log ("Change notifications: Per user", Filter_Roles::translator ());
  for (const auto & user : users) {

    // Total changes made by this user.
    int change_count {0};
    
    // Go through the Bibles changed by the current user.
    vector <string> bibles = database_modifications.getUserBibles (user);
    for (const auto & bible : bibles) {
      
      // Body of the email to be sent.
      string email = "<p>" + translate("You have entered the changes below in a Bible editor.") + " " + translate ("You may check if it made its way into the Bible text.") + "</p>";
      size_t empty_email_length = email.length ();
      
      // Go through the books in that Bible.
      vector <int> books = database_modifications.getUserBooks (user, bible);
      for (auto book : books) {
        
        // Go through the chapters in that book.
        vector <int> chapters = database_modifications.getUserChapters (user, bible, book);
        for (auto chapter : chapters) {
          
          // Get the sets of identifiers for that chapter, and set some variables.
          vector <Database_Modifications_Id> IdSets = database_modifications.getUserIdentifiers (user, bible, book, chapter);
          int reference_new_id {0};
          int new_id {0};
          int last_new_id {0};
          bool restart = true;
          
          // Go through the sets of identifiers.
          for (const auto & id_set : IdSets) {
            
            int oldId {id_set.oldid};
            new_id = id_set.newid;
            
            if (restart) {
              changes_process_identifiers (webserver_request, user, recipients_named_contributors, bible, book, chapter, notification_bibles_per_user, reference_new_id, new_id, email, change_count, modification_time_total, modification_time_count);
              reference_new_id = new_id;
              last_new_id = new_id;
              restart = false;
              continue;
            }
            
            if (oldId == last_new_id) {
              last_new_id = new_id;
            } else {
              restart = true;
            }
          }
          
          // Process the last set of identifiers.
          changes_process_identifiers (webserver_request, user, recipients_named_contributors, bible, book, chapter, notification_bibles_per_user, reference_new_id, new_id, email, change_count, modification_time_total, modification_time_count);
          
        }
      }

      // Check whether there's any email to be sent.
      if (email.length () != empty_email_length) {
        // Send the user email with the user's personal changes if the user opted to receive it.
        if (webserver_request.database_config_user()->getUserUserChangesNotification (user)) {
          string subject = translate("Changes you entered in") + " " + bible;
          if (!client_logic_client_enabled ()) email_schedule (user, subject, email);
        }
      }
    }
    
    // Store change statistics for this user.
    user_change_statistics [user] = change_count;

    // Clear the user's changes in the database.
    database_modifications.clearUserUser (user);
    
    
    // Clear checksum cache.
    webserver_request.database_config_user ()->setUserChangeNotificationsChecksum (user, "");
  }
  
  
  // Generate the notifications, online and by email,
  // for the changes in the Bibles entered by anyone
  // since the previous notifications were generated.
  vector <string> bibles = database_modifications.getTeamDiffBibles ();
  for (const auto & bible : bibles) {
    
    
    string stylesheet = Database_Config_Bible::getExportStylesheet (bible);
    
    
    vector <string> changeNotificationUsers;
    vector <string> all_users = webserver_request.database_users ()->get_users ();
    for (const auto & user : all_users) {
      if (access_bible::read (webserver_request, bible, user)) {
        if (webserver_request.database_config_user()->getUserGenerateChangeNotifications (user)) {
          // The recipient may have set which Bibles to get the change notifications for.
          // This is stored like this:
          // container [user] = list of bibles.
          bool receive {true};
          try {
            const vector <string> & user_bibles = notification_bibles_per_user.at(user);
            receive = in_array(bible, user_bibles);
          } catch (...) {}
          if (receive) {
            changeNotificationUsers.push_back (user);
          }
        }
      }
    }
    all_users.clear ();
    
    
    // The number of changes processed so far for this Bible.
    int processedChangesCount = 0;
    
    
    // The files get stored at http://site.org:<port>/revisions/<Bible>/<date>
    int seconds = filter::date::seconds_since_epoch ();
    string timepath;
    timepath.append (filter::strings::convert_to_string (filter::date::numerical_year (seconds)));
    timepath.append ("-");
    timepath.append (filter::strings::fill (filter::strings::convert_to_string (filter::date::numerical_month (seconds)), 2, '0'));
    timepath.append ("-");
    timepath.append (filter::strings::fill (filter::strings::convert_to_string (filter::date::numerical_month_day (seconds)), 2, '0'));
    timepath.append (" ");
    timepath.append (filter::strings::fill (filter::strings::convert_to_string (filter::date::numerical_hour (seconds)), 2, '0'));
    timepath.append (":");
    timepath.append (filter::strings::fill (filter::strings::convert_to_string (filter::date::numerical_minute (seconds)), 2, '0'));
    timepath.append (":");
    timepath.append (filter::strings::fill (filter::strings::convert_to_string (filter::date::numerical_second (seconds)), 2, '0'));
    string directory = filter_url_create_root_path ({"revisions", bible, timepath});
    filter_url_mkdir (directory);
    
    
    // Produce the USFM and html files.
    filter_diff_produce_verse_level (bible, directory);
    
    
    // Create online page with changed verses.
    string versesoutputfile = filter_url_create_path ({directory, "changed_verses.html"});
    filter_diff_run_file (filter_url_create_path ({directory, "verses_old.txt"}), filter_url_create_path ({directory, "verses_new.txt"}), versesoutputfile);
    
    
    // Storage for body of the email with the changes.
    vector <string> email_changes {};
    
    
    // Generate the online change notifications.
    vector <int> books = database_modifications.getTeamDiffBooks (bible);
    for (auto book : books) {
      vector <int> chapters = database_modifications.getTeamDiffChapters (bible, book);
      for (auto chapter : chapters) {
        Database_Logs::log ("Change notifications: " + bible + " " + filter_passage_display (book, chapter, ""), Filter_Roles::translator ());
        string old_chapter_usfm = database_modifications.getTeamDiff (bible, book, chapter);
        string new_chapter_usfm = webserver_request.database_bibles()->get_chapter (bible, book, chapter);
        vector <int> old_verse_numbers = filter::usfm::get_verse_numbers (old_chapter_usfm);
        vector <int> new_verse_numbers = filter::usfm::get_verse_numbers (new_chapter_usfm);
        vector <int> verses = old_verse_numbers;
        verses.insert (verses.end (), new_verse_numbers.begin (), new_verse_numbers.end ());
        verses = filter::strings::array_unique (verses);
        sort (verses.begin (), verses.end());
        for (auto verse : verses) {
          string old_verse_usfm = filter::usfm::get_verse_text (old_chapter_usfm, verse);
          string new_verse_usfm = filter::usfm::get_verse_text (new_chapter_usfm, verse);
          if (old_verse_usfm != new_verse_usfm) {
            processedChangesCount++;
            // In case of too many change notifications, processing them would take too much time, so take a few shortcuts.
            string old_html = "<p>" + old_verse_usfm + "</p>";
            string new_html = "<p>" + new_verse_usfm + "</p>";
            string old_text = old_verse_usfm;
            string new_text = new_verse_usfm;
            if (processedChangesCount < 800) {
              Filter_Text filter_text_old = Filter_Text (bible);
              Filter_Text filter_text_new = Filter_Text (bible);
              filter_text_old.html_text_standard = new HtmlText ("");
              filter_text_new.html_text_standard = new HtmlText ("");
              filter_text_old.text_text = new Text_Text ();
              filter_text_new.text_text = new Text_Text ();
              filter_text_old.add_usfm_code (old_verse_usfm);
              filter_text_new.add_usfm_code (new_verse_usfm);
              filter_text_old.run (stylesheet);
              filter_text_new.run (stylesheet);
              old_html = filter_text_old.html_text_standard->get_inner_html ();
              new_html = filter_text_new.html_text_standard->get_inner_html ();
              old_text = filter_text_old.text_text->get ();
              new_text = filter_text_new.text_text->get ();
            }
            string modification = filter_diff_diff (old_text, new_text);
            database_modifications.recordNotification (changeNotificationUsers, changes_bible_category (), bible, book, chapter, verse, old_html, modification, new_html);
            string passage = filter_passage_display (book, chapter, filter::strings::convert_to_string (verse))   + ": ";
            if (old_text != new_text) {
              email_changes.push_back (passage  + modification);
            } else {
              email_changes.push_back (translate ("The following passage has no change in the text.") + " " + translate ("Only the formatting was changed.") + " " + translate ("The USFM code is given for reference."));
              email_changes.push_back (passage);
              email_changes.push_back (translate ("Old code:") + " " + old_verse_usfm);
              email_changes.push_back (translate ("New code:") + " " + new_verse_usfm);
            }
          }
        }
        // Delete the diff data for this chapter, for two reasons:
        // 1. New diffs for this chapter can be stored straightaway.
        // 2. In case of large amounts of diff data, and this function gets killed,
        //    then the next time it runs again, it will continue from where it was killed.
        database_modifications.deleteTeamDiffChapter (bible, book, chapter);
      }
    }
    
    
    // Email the changes to the subscribed users.
    if (!email_changes.empty ()) {
      // Split large emails up into parts.
      // The size of the parts has been found by checking the maximum size that the emailer will send,
      // then each part should remain well below that maximum size.
      // The size was reduced even more later on:
      // https://github.com/bibledit/cloud/issues/727
      vector <string> bodies {};
      int counter {0};
      string body {};
      for (const auto & line : email_changes) {
        body.append ("<div>");
        body.append (line);
        body.append ("</div>\n");
        counter++;
        if (counter >= 150) {
          bodies.push_back (body);
          body.clear ();
          counter = 0;
        }
      }
      if (!body.empty ()) bodies.push_back (body);
      for (size_t b = 0; b < bodies.size (); b++) {
        string subject = translate("Recent changes:") + " " + bible;
        if (bodies.size () > 1) {
          subject.append (" (" + filter::strings::convert_to_string (b + 1) + "/" + filter::strings::convert_to_string (bodies.size ()) + ")");
        }
        vector <string> all_users_2 = webserver_request.database_users ()->get_users ();
        for (auto & user : all_users_2) {
          if (webserver_request.database_config_user()->getUserBibleChangesNotification (user)) {
            if (access_bible::read (webserver_request, bible, user)) {
              if (!client_logic_client_enabled ()) {
                email_schedule (user, subject, bodies[b]);
              }
            }
          }
        }
      }
    }
  }

  
  // Index the data and remove expired notifications.
  Database_Logs::log ("Change notifications: Indexing", Filter_Roles::translator ());
  database_modifications.indexTrimAllNotifications ();

  
  // Remove expired downloadable revisions.
  string directory = filter_url_create_root_path ({"revisions"});
  int now = filter::date::seconds_since_epoch ();
  bibles = filter_url_scandir (directory);
  for (const auto & bible : bibles) {
    string folder = filter_url_create_path ({directory, bible});
    int time = filter_url_file_modification_time (folder);
    int days = (now - time) / 86400;
    if (days > 31) {
      filter_url_rmdir (folder);
    } else {
      vector <string> revisions = filter_url_scandir (folder);
      for (auto & revision : revisions) {
        string path = filter_url_create_path ({folder, revision});
        int time2 = filter_url_file_modification_time (path);
        int days2 = (now - time2) / 86400;
        if (days2 > 31) {
          filter_url_rmdir (path);
          Database_Logs::log ("Removing expired downloadable revision notification: " + bible + " " + revision, Filter_Roles::translator ());
        }
      }
    }
  }
  
  
  // Clear checksum caches.
  users = webserver_request.database_users ()->get_users ();
  for (const auto & user : users) {
    webserver_request.database_config_user ()->setUserChangeNotificationsChecksum (user, "");
  }
  
  
  // Vacuum the modifications index, as it might have been updated.
  database_modifications.vacuum ();
  
  
  // Make the notifications available again to clients.
  config_globals_change_notifications_available = true;

  
#ifdef HAVE_CLOUD
  // Store the statistics in the database.
  if (modification_time_count) {
    // Take average timestamp of all timestamps.
    int timestamp = static_cast <int> (round (modification_time_total / static_cast<float>(modification_time_count)));
    for (auto & element : user_change_statistics) {
      // Store dated change statistics per user.
      string user = element.first;
      int count = element.second;
      Database_Statistics::store_changes (timestamp, user, count);
    }
  }
#endif
  

  Database_Logs::log ("Change notifications: Ready", Filter_Roles::translator ());
}
