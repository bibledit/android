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


#include <fonts/logic.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/config/bible.h>


string Fonts_Logic::folder ()
{
  return filter_url_create_root_path ({"fonts"});
}


vector <string> Fonts_Logic::getFonts ()
{
  vector <string> files = filter_url_scandir (folder());
  vector <string> fonts;
  for (auto & file : files) {
    string suffix = filter_url_get_extension (file);
    if (suffix == "txt") continue;
    if (suffix == "html") continue;
    if (suffix == "h") continue;
    if (suffix == "cpp") continue;
    if (suffix == "o") continue;
    fonts.push_back (file);
  }
  return fonts;
}


bool Fonts_Logic::font_exists (string font)
{
  string path = filter_url_create_path ({folder (), font});
  return file_or_dir_exists (path);
}


string Fonts_Logic::get_font_path (string font)
{
  // Case of no font.
  if (font == string()) return string();
  
  // Case when the font exists within Bibledit.
  if (font_exists (font)) {
    return filter_url_create_path ({"../fonts", font});
  }
  
  // Case when the font is available from the browser independent of Bibledit.
  if (filter_url_basename (font) == font) {
    return font;
  }
  
  // Font is on external location.
  return font;
}


void Fonts_Logic::erase (string font)
{
  string path = filter_url_create_path ({folder (), font});
  filter_url_unlink (path);
}



// When a font is set for a Bible in Bibledit Cloud, this becomes the default font for the clients.
// Ahd when the client sets its own font, this font will be taken instead.
string Fonts_Logic::get_text_font (string bible)
{
  string font = Database_Config_Bible::getTextFont (bible);
#ifdef HAVE_CLIENT
  string client_font = Database_Config_Bible::getTextFontClient (bible);
  if (!client_font.empty ()) {
    font = client_font;
  }
#endif
  return font;
}


// Returns true if the $font path has a font suffix.
bool Fonts_Logic::is_font (string suffix)
{
  return (suffix == "ttf")
      || (suffix == "otf")
      || (suffix == "otf")
      || (suffix == "woff");
}
