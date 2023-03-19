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


#include <filter/indonesian.h>
#include <database/logs.h>
#include <database/books.h>
#include <database/bibles.h>
#include <search/logic.h>
#include <book/create.h>
#include <locale/translate.h>
using namespace std;


namespace filter::indonesian {

string ourtranslation ()
{
  return "AlkitabKita";
}


string mytranslation (const string & user)
{
  string name = "Terjemahanku";
  if (!user.empty()) {
    name.append (" ");
    name.append (user);
  }
  return name;
}


}
