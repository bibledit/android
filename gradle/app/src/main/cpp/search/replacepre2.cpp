/*
 Copyright (©) 2003-2023 Teus Benschop.
 
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


#include <search/replacepre2.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <locale/logic.h>
#include <database/config/general.h>
#include <search/logic.h>
#include <access/bible.h>
using namespace std;


string search_replacepre2_url ()
{
  return "search/replacepre2";
}


bool search_replacepre2_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return write;
}


string search_replacepre2 (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  
  
  string siteUrl = config::logic::site_url (webserver_request);
  
  
  // Get search variables from the query.
  string searchfor = request->query ["q"];
  string replacewith = request->query ["r"];
  bool casesensitive = (request->query ["c"] == "true");
  string id = request->query ["id"];
  bool searchplain = (request->query ["p"] == "true");
  
  
  // Get the Bible and passage for this identifier.
  Passage details = Passage::decode (id);
  string bible = details.m_bible;
  int book = details.m_book;
  int chapter = details.m_chapter;
  string verse = details.m_verse;
  
  
  // Get the plain text or the USFM.
  string text;
  if (searchplain) {
    text = search_logic_get_bible_verse_text (bible, book, chapter, filter::strings::convert_to_int (verse));
  } else {
    text = search_logic_get_bible_verse_usfm (bible, book, chapter, filter::strings::convert_to_int (verse));
  }
  
  // Clickable passage.
  string link = filter_passage_link_for_opening_editor_at (book, chapter, verse);
  
  
  string oldtext = filter::strings::markup_words ({searchfor}, text);
  

  string newtext (text);
  if (casesensitive) {
    newtext = filter::strings::replace (searchfor, replacewith, newtext);
  } else {
    vector <string> needles = filter::strings::search_needles (searchfor, text);
    for (auto & needle : needles) {
      newtext = filter::strings::replace (needle, replacewith, newtext);
    }
  }
  if (replacewith != "") newtext = filter::strings::markup_words ({replacewith}, newtext);
  
  
  // Check whether the user has write access to the book.
  string user = request->session_logic ()->currentUser ();
  bool write = access_bible::book_write (webserver_request, user, bible, book);

  
  // Create output.
  string output;
  output.append ("<div id=\"" + id + "\">\n");
  output.append ("<p>");
  if (write) output.append ("<a href=\"replace\"> ✔ </a> <a href=\"delete\">" + filter::strings::emoji_wastebasket () + "</a> ");
  output.append (link);
  output.append ("</p>\n");
  output.append ("<p>" + oldtext + "</p>\n");
  output.append ("<p>");
  if (write) output.append (newtext);
  else output.append (locale_logic_text_no_privileges_modify_book ());
  output.append ("</p>\n");
  output.append ("</div>\n");
  
  
  // Output to browser.
  return output;
}
