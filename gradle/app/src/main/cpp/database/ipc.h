/*
Copyright (©) 2003-2022 Teus Benschop.

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


#pragma once


#include <config/libraries.h>


struct Database_Ipc_Item
{
  string file {};
  int rowid {0};
  string user {};
  string channel {};
  string command {};
};


struct Database_Ipc_Message
{
  int id {0};
  string channel {};
  string command {};
  string message {};
};


class Database_Ipc
{
public:
  Database_Ipc (void * webserver_request_in);
  void trim ();
  void storeMessage (string user, string channel, string command, string message);
  Database_Ipc_Message retrieveMessage (int id, string user, string channel, string command);
  void deleteMessage (int id);
  string getFocus ();
  Database_Ipc_Message getNote ();
  bool getNotesAlive ();
private:
  void * webserver_request {nullptr};
  string folder ();
  string file (string file);
  vector <Database_Ipc_Item> readData ();
  void writeRecord (int rowid, string user, string channel, string command, string message);
  int getNextId (const vector <Database_Ipc_Item> & data);
};


