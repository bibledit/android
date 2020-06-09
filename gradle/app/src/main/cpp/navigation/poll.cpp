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

#include <navigation/poll.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <navigation/passage.h>
#include <ipc/focus.h>


string navigation_poll_url ()
{
  return "navigation/poll";
}


bool navigation_poll_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


string navigation_poll (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  int book = Ipc_Focus::getBook (request);
  int chapter = Ipc_Focus::getChapter (request);
  int verse = Ipc_Focus::getVerse (request);
  vector <string> passage;
  passage.push_back (convert_to_string (book));
  passage.push_back (convert_to_string (chapter));
  passage.push_back (convert_to_string (verse));
  return filter_string_implode (passage, "\n");
}

