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


#include <editusfm/focus.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <webserver/request.h>
#include <ipc/focus.h>
#include <access/bible.h>
using namespace std;


string editusfm_focus_url ()
{
  return "editusfm/focus";
}


bool editusfm_focus_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return read;
}


// Returns two numerical positions: A starting one, and an ending one.
// These two are for positioning the caret in the editor.
// The caret should be at or be moved to a position between these two.
string editusfm_focus (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  string bible = request->query ["bible"];
  int book = convert_to_int (request->query ["book"]);
  int chapter = convert_to_int (request->query ["chapter"]);
  string usfm = request->database_bibles()->getChapter (bible, book, chapter);
  int verse = Ipc_Focus::getVerse (request);
  int startingOffset = filter::usfm::versenumber_to_offset (usfm, verse);
  int endingOffset = startingOffset;
  // The following deals with a combined verse.
  for (int i = 1; i < 25; i++) {
    if (startingOffset == endingOffset) {
      endingOffset = filter::usfm::versenumber_to_offset (usfm, verse + i);
      if (endingOffset > startingOffset) endingOffset--;
    }
  }
  string data = convert_to_string (startingOffset) + " " + convert_to_string (endingOffset);
  return data;
}

