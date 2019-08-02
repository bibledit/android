/*
Copyright (©) 2003-2019 Teus Benschop.

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


#include <olb/text.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/url.h>
#include <database/books.h>


// Class for creating input files for the Online Bible compiler.


OnlineBible_Text::OnlineBible_Text ()
{
}


void OnlineBible_Text::storeData ()
{
  if (lineLoaded) output.push_back (filter_string_trim (currentLine));
  currentLine.clear();
  lineLoaded = false;
}


void OnlineBible_Text::newVerse (int bookIdentifier, int chapterNumber, int verseNumber)
{
  storeData ();
  // Store passage and any text only in case the book is valid,
  // and the chapter and verse are non-zero.
  string book = Database_Books::getOnlinebibleFromId (bookIdentifier);
  if (!book.empty()) {
    if (chapterNumber > 0) {
      if (verseNumber > 0) {
        output.push_back ("$$$ " + book + " " + convert_to_string (chapterNumber) + ":" + convert_to_string (verseNumber));
        currentLine.clear ();
        lineLoaded = true;
      }
    }
  }
}



// This function adds $text to the current line.
void OnlineBible_Text::addText (string text)
{
  if (lineLoaded) currentLine += text;
}


// This function adds a note to the current paragraph.
void OnlineBible_Text::addNote ()
{
  addText ("{");
}


// This function closes the current footnote.
void OnlineBible_Text::closeCurrentNote ()
{
  addText ("}");
}


// This saves the data to file $name: the name of the file to save to.
void OnlineBible_Text::save (string name)
{
  storeData ();
  string data = filter_string_implode (output, "\n");
  filter_url_file_put_contents (name, data);
}