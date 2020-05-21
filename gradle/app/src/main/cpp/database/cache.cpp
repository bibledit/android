/*
Copyright (©) 2003-2020 Teus Benschop.

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


#include <database/cache.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/date.h>
#include <filter/shell.h>
#include <database/sqlite.h>
#include <database/logs.h>
#include <database/logic.h>


// Databases resilience:
// They contain cached data.
// Rarely written to.
// Often read from.


string Database_Cache::fragment ()
{
  return "cache_resource_";
}


string Database_Cache::path (string resource, int book)
{
  return filter_url_create_path (database_logic_databases (), filename (filter_url_urlencode (resource), book) + database_sqlite_suffix ());
}


string Database_Cache::filename (string resource, int book)
{
  // Name of the database for this resource.
  resource = filter_url_clean_filename (resource);
  string book_fragment;
  if (book) {
    book_fragment = "_" + convert_to_string (book);
  }
  return fragment () + resource + book_fragment;
}


void Database_Cache::create (string resource, int book)
{
  SqliteDatabase sql = SqliteDatabase (filename (resource, book));

  sql.add ("CREATE TABLE IF NOT EXISTS cache (chapter integer, verse integer, value text);");
  sql.execute ();
  
  sql.clear ();
  
  sql.add ("CREATE TABLE IF NOT EXISTS ready (ready boolean);");
  sql.execute ();
}


void Database_Cache::remove (string resource)
{
  for (int book = 0; book < 100; book++) {
    remove (resource, book);
  }
}


void Database_Cache::remove (string resource, int book)
{
  string file = database_sqlite_file (filename (resource, book));
  if (file_or_dir_exists (file)) {
    filter_url_unlink (file);
  }
}


// Returns true if the cache for the $resource exists.
bool Database_Cache::exists (string resource)
{
  for (int book = 0; book < 100; book++) {
    if (exists (resource, book)) return true;
  }
  return false;
}


// Returns true if the cache for the $resource $book exists.
bool Database_Cache::exists (string resource, int book)
{
  string file = database_sqlite_file (filename (resource, book));
  return file_or_dir_exists (file);
}


// Returns true if a cached value for $resource/book/chapter/verse exists.
bool Database_Cache::exists (string resource, int book, int chapter, int verse)
{
  // If the the book-based cache exists, check existence from there.
  if (exists (resource, book)) {
    SqliteDatabase sql = SqliteDatabase (filename (resource, book));
    sql.add ("SELECT count(*) FROM cache WHERE chapter = ");
    sql.add (chapter);
    sql.add ("AND verse = ");
    sql.add (verse);
    sql.add (";");
    vector <string> result = sql.query () ["count(*)"];
    int count = 0;
    if (!result.empty ()) count = convert_to_int (result [0]);
    return (count > 0);
  }
  // Else if the previous cache layout exists, check that.
  if (exists (resource, 0)) {
    SqliteDatabase sql = SqliteDatabase (filename (resource, 0));
    sql.add ("SELECT count(*) FROM cache WHERE book =");
    sql.add (book);
    sql.add ("AND chapter = ");
    sql.add (chapter);
    sql.add ("AND verse = ");
    sql.add (verse);
    sql.add (";");
    vector <string> result = sql.query () ["count(*)"];
    int count = 0;
    if (!result.empty ()) count = convert_to_int (result [0]);
    return (count > 0);
  }
  // Nothing exists.
  return false;
}


// Caches a value.
void Database_Cache::cache (string resource, int book, int chapter, int verse, string value)
{
  SqliteDatabase sql = SqliteDatabase (filename (resource, book));

  sql.clear ();
  sql.add ("DELETE FROM cache WHERE chapter = ");
  sql.add (chapter);
  sql.add ("AND verse = ");
  sql.add (verse);
  sql.add (";");
  sql.execute ();
  
  sql.clear ();
  sql.add ("INSERT INTO cache VALUES (");
  sql.add (chapter);
  sql.add (",");
  sql.add (verse);
  sql.add (",");
  sql.add (value);
  sql.add (");");
  sql.execute ();
}


// Retrieves a cached value.
string Database_Cache::retrieve (string resource, int book, int chapter, int verse)
{
  // If the the book-based cache exists, retrieve it from there.
  if (exists (resource, book)) {
    SqliteDatabase sql = SqliteDatabase (filename (resource, book));
    sql.add ("SELECT value FROM cache WHERE chapter = ");
    sql.add (chapter);
    sql.add ("AND verse = ");
    sql.add (verse);
    sql.add (";");
    vector <string> result = sql.query () ["value"];
    if (result.empty ()) return "";
    return result [0];
  }
  // Else if the previous cache layout exists, retrieve it from there.
  if (exists (resource, 0)) {
    SqliteDatabase sql = SqliteDatabase (filename (resource, 0));
    sql.add ("SELECT value FROM cache WHERE book =");
    sql.add (book);
    sql.add ("AND chapter = ");
    sql.add (chapter);
    sql.add ("AND verse = ");
    sql.add (verse);
    sql.add (";");
    vector <string> result = sql.query () ["value"];
    if (result.empty ()) return "";
    return result [0];
  }
  return "";
}


// Returns how many element are in cache $resource.
int Database_Cache::count (string resource)
{
  int count = 0;
  // Book 0 is for the old layout. Book 1++ is for the new layout.
  for (int book = 0; book < 100; book++) {
    if (exists (resource, book)) {
      count ++;
    }
  }
  return count;
}


// Return true if the database has loaded all its expected content.
bool Database_Cache::ready (string resource, int book)
{
  SqliteDatabase sql = SqliteDatabase (filename (resource, book));
  sql.add ("SELECT ready FROM ready;");
  vector <string> result = sql.query () ["ready"];
  for (auto ready : result) {
    return convert_to_bool (ready);
  }
  return false;
}


// Sets the 'ready' flag in the database.
void Database_Cache::ready (string resource, int book, bool ready)
{
  SqliteDatabase sql = SqliteDatabase (filename (resource, book));
  
  sql.clear ();
  sql.add ("DELETE FROM ready;");
  sql.execute ();
  
  sql.clear ();
  sql.add ("INSERT INTO ready VALUES (");
  sql.add (ready);
  sql.add (");");
  sql.execute ();
}


int Database_Cache::size (string resource, int book)
{
  string file = database_sqlite_file (filename (resource, book));
  return filter_url_filesize (file);
}


string database_cache_full_path (string file)
{
  return filter_url_create_root_path (database_logic_databases (), "cache", file);
}


// The purpose of splitting the file up into paths is
// to avoid that the cache folder would contain too many files
// and so would become slow.
string database_cache_split_file (string file)
{
  if (file.size () > 9) file.insert (9, "/");
  if (file.size () > 18) file.insert (18, "/");
  if (file.size () > 27) file.insert (27, "/");
  file.append (".txt");
  return file;
}


bool database_filebased_cache_exists (string schema)
{
  schema = filter_url_clean_filename (schema);
  schema = database_cache_split_file (schema);
  schema = database_cache_full_path (schema);
  return file_or_dir_exists (schema);
}


void database_filebased_cache_put (string schema, string contents)
{
  schema = filter_url_clean_filename (schema);
  schema = database_cache_split_file (schema);
  schema = database_cache_full_path (schema);
  string path = filter_url_dirname (schema);
  if (!file_or_dir_exists (path)) filter_url_mkdir (path);
  filter_url_file_put_contents (schema, contents);
}


string database_filebased_cache_get (string schema)
{
  schema = filter_url_clean_filename (schema);
  schema = database_cache_split_file (schema);
  schema = database_cache_full_path (schema);
  return filter_url_file_get_contents (schema);
}


void database_filebased_cache_remove (string schema)
{
  schema = filter_url_clean_filename (schema);
  schema = database_cache_split_file (schema);
  schema = database_cache_full_path (schema);
  filter_url_unlink (schema);
}


// Deletes expired cached items.
void database_cache_trim (bool clear)
{
  if (clear) Database_Logs::log ("Clearing cache");

  string output, error;

  // The directory that contains the file-based cache files.
  string path = database_cache_full_path ("");

  // There have been instances that the cache takes up 4, 5, or 6 Gbytes in the Cloud.
  // If the cache is left untrimmed, the size can be even larger.
  // This leads to errors when the disk runs out of space.
  // Therefore it's good to remove cached files older than a couple of hours.
  string minutes = "+120";

  // Handle clearing the cache immediately.
  if (clear) minutes = "+0";

  // Remove files that have not been modified for x minutes.
  // It uses a Linux shell command.
  // This can be done because it runs on the server only.
  // It used to do "cd /path/to/cache; find /path/to/cache ...".
  // This could remove the folder "cache" too.
  // After this folder was removed, no caching could take place anymore.
  // https://github.com/bibledit/cloud/issues/366
  // This was not visible on macOS, but it was visible on Linux.
  // The fix is to do "cd /path/to/cache; find . ...".
  output.clear ();
  error.clear ();
  filter_shell_run (path, "find", {".", "-amin", minutes, "-delete"}, &output, &error);
  if (!output.empty ()) Database_Logs::log (output);
  if (!error.empty ()) Database_Logs::log (error);
  
  // Remove empty directories.
  output.clear ();
  error.clear ();
  filter_shell_run (path, "find", {".", "-type", "d", "-empty", "-delete"}, &output, &error);
  if (!output.empty ()) Database_Logs::log (output);
  if (!error.empty ()) Database_Logs::log (error);
  
  // The directory that contains the database-based cache files.
  path = filter_url_create_root_path (database_logic_databases ());

  // The space these database-based cache files use.
  output.clear ();
  error.clear ();
  int megabytes = 0;
  filter_shell_run ("cd " + path + "; du -csm " + Database_Cache::fragment () + "*", output);
  vector <string> lines = filter_string_explode (output, '\n');
  for (auto line : lines) {
    megabytes = convert_to_int (line);
  }

  // The number of days to keep cached data depends on the size the cache takes.
  // There have been instances that the cache takes up 4, 5, or 6 Gbytes in the Cloud.
  // This can be even more.
  // This may lead to errors when the disk runs out of space.
  // Therefore it's good to limit large caches more speedy.
  string days = "+5";
  if (megabytes >=  50) days = "+4";
  if (megabytes >= 100) days = "+3";
  if (megabytes >= 150) days = "+2";
  if (megabytes >= 200) days = "+1";

  // Handle clearing the cache immediately.
  if (clear) days = "0";

  // Remove database-based cached files that have not been modified for x days.
  output.clear ();
  error.clear ();
  filter_shell_run (path, "find", {path, "-name", Database_Cache::fragment () + "*", "-atime", days, "-delete"}, &output, &error);
  if (!output.empty ()) Database_Logs::log (output);
  if (!error.empty ()) Database_Logs::log (error);
  
  if (clear) Database_Logs::log ("Ready clearing  cache");
}
