/*
 Copyright (©) 2003-2021 Teus Benschop.
 
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


#ifndef INCLUDED_CHECK_PAIRS_H
#define INCLUDED_CHECK_PAIRS_H


#include <config/libraries.h>


class Checks_Pairs
{
public:
  static void run (const string & bible, int book, int chapter,
                   const map <int, string> & texts,
                   const vector <pair <string, string> > & pairs,
                   bool french_citation_style);
private:
  static string match (const string & character, const vector <pair <string, string> > & pairs);
};


#endif
