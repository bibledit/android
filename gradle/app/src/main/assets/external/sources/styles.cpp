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


#include <sources/styles.h>
#include <filter/string.h>
#include <filter/url.h>


string sources_style_parse_create_c_comment (string key)
{
  return "/* " + key + " */";
}


string sources_style_parse_generate_entry (string key, string value, bool quote)
{
  string result;
  result.append ("    ");
  result.append (sources_style_parse_create_c_comment (key));
  result.append (" ");
  if (quote) result.append ("\"");
  result.append (value);
  if (quote) result.append ("\"");
  result.append (",");
  return result;
}


bool sources_style_parse_cpp_element (string & line, string signature)
{
  signature = sources_style_parse_create_c_comment (signature);
  size_t pos = line.find (signature);
  if (pos == string::npos) return false;
  line.erase (0, pos + signature.length ());
  line = filter_string_trim (line);
  if (line [line.size () - 1] == ',') line = line.erase (line.size () - 1);
  if (line [line.size () - 1] == '"') line = line.erase (line.size () - 1);
  if (line [0] == '"') line.erase (0, 1);
  return true;
}


// Parses usfm.sty (see https://github.com/ubsicap/usfm) and updates values in the default stylesheet.
void sources_styles_parse ()
{

  // Read the C++ source files with the default style definitions.
  string cpp_path = filter_url_create_root_path ("database", "styles.cpp");
  string contents = filter_url_file_get_contents (cpp_path);
  string cpp_start = "style_record styles_table [] =";
  string cpp_end = "};";
  size_t pos = contents.find (cpp_start);
  if (pos == string::npos) {
    cout << "Cannot find default styles signature: " << cpp_start << endl;
    return;
  }
  contents.erase (0, pos + cpp_start.size() + 1);
  pos = contents.find (cpp_end);
  if (pos == string::npos) {
    cout << "Cannot find default styles signature: " << cpp_end << endl;
    return;
  }
  contents.erase (pos);
  vector <string> cpp_lines = filter_string_explode (contents, '\n');

  // Parser signatures for the C++ source code.
  const char * marker_key = "marker";
  const char * name_key = "name";
  const char * info_key = "info";
  const char * category_key = "category";
  const char * type_key = "type";
  const char * subtype_key = "subtype";
  const char * fontsize_key = "fontsize";
  const char * italic_key = "italic";
  const char * bold_key = "bold";
  const char * underline_key = "underline";
  const char * smallcaps_key = "smallcaps";
  const char * superscript_key = "superscript";
  const char * justification_key = "justification";
  const char * spacebefore_key = "spacebefore";
  const char * spaceafter_key = "spaceafter";
  const char * leftmargin_key = "leftmargin";
  const char * rightmargin_key = "rightmargin";
  const char * firstlineindent_key = "firstlineindent";
  const char * spancolumns_key = "spancolumns";
  const char * color_key = "color";
  const char * print_key = "print";
  const char * userbool1_key = "userbool1";
  const char * userbool2_key = "userbool2";
  const char * userbool3_key = "userbool3";
  const char * userint1_key = "userint1";
  const char * userint2_key = "userint2";
  const char * userint3_key = "userint3";
  const char * userstring1_key = "userstring1";
  const char * userstring2_key = "userstring2";
  const char * userstring3_key = "userstring3";
  const char * backgroundcolor_key = "backgroundcolor";
  
  // Parse the default style definitions.
  typedef map <string, string> definition_type;
  definition_type style_definition;
  map <string, definition_type> style_definitions;
  string marker;
  for (auto line : cpp_lines) {
    if (sources_style_parse_cpp_element (line, marker_key)) {
      marker = line;
      style_definitions [marker] [marker_key] = marker;
    }
    if (sources_style_parse_cpp_element (line, name_key)) {
      style_definitions [marker] [name_key] = line;
    }
    if (sources_style_parse_cpp_element (line, info_key)) {
      style_definitions [marker] [info_key] = line;
    }
    if (sources_style_parse_cpp_element (line, category_key)) {
      style_definitions [marker] [category_key] = line;
    }
    if (sources_style_parse_cpp_element (line, type_key)) {
      style_definitions [marker] [type_key] = line;
    }
    if (sources_style_parse_cpp_element (line, subtype_key)) {
      style_definitions [marker] [subtype_key] = line;
    }
    if (sources_style_parse_cpp_element (line, fontsize_key)) {
      style_definitions [marker] [fontsize_key] = line;
    }
    if (sources_style_parse_cpp_element (line, italic_key)) {
      style_definitions [marker] [italic_key] = line;
    }
    if (sources_style_parse_cpp_element (line, bold_key)) {
      style_definitions [marker] [bold_key] = line;
    }
    if (sources_style_parse_cpp_element (line, underline_key)) {
      style_definitions [marker] [underline_key] = line;
    }
    if (sources_style_parse_cpp_element (line, smallcaps_key)) {
      style_definitions [marker] [smallcaps_key] = line;
    }
    if (sources_style_parse_cpp_element (line, superscript_key)) {
      style_definitions [marker] [superscript_key] = line;
    }
    if (sources_style_parse_cpp_element (line, justification_key)) {
      style_definitions [marker] [justification_key] = line;
    }
    if (sources_style_parse_cpp_element (line, spacebefore_key)) {
      style_definitions [marker] [spacebefore_key] = line;
    }
    if (sources_style_parse_cpp_element (line, spaceafter_key)) {
      style_definitions [marker] [spaceafter_key] = line;
    }
    if (sources_style_parse_cpp_element (line, leftmargin_key)) {
      style_definitions [marker] [leftmargin_key] = line;
    }
    if (sources_style_parse_cpp_element (line, rightmargin_key)) {
      style_definitions [marker] [rightmargin_key] = line;
    }
    if (sources_style_parse_cpp_element (line, firstlineindent_key)) {
      style_definitions [marker] [firstlineindent_key] = line;
    }
    if (sources_style_parse_cpp_element (line, spancolumns_key)) {
      style_definitions [marker] [spancolumns_key] = line;
    }
    if (sources_style_parse_cpp_element (line, color_key)) {
      style_definitions [marker] [color_key] = line;
    }
    if (sources_style_parse_cpp_element (line, print_key)) {
      style_definitions [marker] [print_key] = line;
    }
    if (sources_style_parse_cpp_element (line, userbool1_key)) {
      style_definitions [marker] [userbool1_key] = line;
    }
    if (sources_style_parse_cpp_element (line, userbool2_key)) {
      style_definitions [marker] [userbool2_key] = line;
    }
    if (sources_style_parse_cpp_element (line, userbool3_key)) {
      style_definitions [marker] [userbool3_key] = line;
    }
    if (sources_style_parse_cpp_element (line, userint1_key)) {
      style_definitions [marker] [userint1_key] = line;
    }
    if (sources_style_parse_cpp_element (line, userint2_key)) {
      style_definitions [marker] [userint2_key] = line;
    }
    if (sources_style_parse_cpp_element (line, userint3_key)) {
      style_definitions [marker] [userint3_key] = line;
    }
    if (sources_style_parse_cpp_element (line, userstring1_key)) {
      style_definitions [marker] [userstring1_key] = line;
    }
    if (sources_style_parse_cpp_element (line, userstring2_key)) {
      style_definitions [marker] [userstring2_key] = line;
    }
    if (sources_style_parse_cpp_element (line, userstring3_key)) {
      style_definitions [marker] [userstring3_key] = line;
    }
    if (sources_style_parse_cpp_element (line, backgroundcolor_key)) {
      style_definitions [marker] [backgroundcolor_key] = line;
    }
  }
  
  // Read the default stylesheet for Paratext projects.
  string path = filter_url_create_root_path ("sources", "usfm.sty");
  contents = filter_url_file_get_contents (path);
  vector <string> paratext_lines = filter_string_explode (contents, '\n');
  
  // Parse state variables.
  string paratext_marker;
  vector <string> non_existing_markers;

  // Parser signatures for the Paratext stylesheet usfm.sty.
  string backslash_marker = "\\Marker ";
  string backslash_fontsize = "\\FontSize ";
  string backslash_leftmargin = "\\LeftMargin ";
  string backslash_rightmargin = "\\RightMargin ";
  string backslash_firstlineindent = "\\FirstLineIndent ";
  string backslash_spacebefore = "\\SpaceBefore ";
  string backslash_spaceafter = "\\SpaceAfter ";
  
  // Parse the stylesheet.
  for (auto paratext_line : paratext_lines) {
    
    // An empty line: End of style block reached.
    paratext_line = filter_string_trim (paratext_line);
    if (paratext_line.empty ()) {
      paratext_marker.clear ();
      continue;
    }
    
    // Look for the start of a style block trough e.g. "\Marker id".
    if (paratext_line.find (backslash_marker) == 0) {
      paratext_line.erase (0, backslash_marker.length ());
      string marker = filter_string_trim (paratext_line);
      // Skip markers in the z-area.
      if (marker [0] == 'z') continue;
      // A new style block starts here.
      paratext_marker = marker;
      continue;
    }

    // No marker: nothing to parse.
    if (paratext_marker.empty ()) continue;
    
    // Check whether the marker in usfm.sty exists in C++.
    if (style_definitions.count (paratext_marker) == 0) {
      if (in_array (paratext_marker, non_existing_markers)) continue;
      cout << "Marker " << paratext_marker << " exists in usfm.sty but not in Bibledit" << endl;
      non_existing_markers.push_back (paratext_marker);
      continue;
    }
    
    // Read and import the font size.
    if (paratext_line.find (backslash_fontsize) == 0) {
      paratext_line.erase (0, backslash_fontsize.length());
      string fontsize = filter_string_trim (paratext_line);
      style_definitions [paratext_marker] [fontsize_key] = fontsize;
      continue;
    }
    
    // Read and import the left margin.
    if (paratext_line.find (backslash_leftmargin) == 0) {
      paratext_line.erase (0, backslash_leftmargin.length());
      string inches = filter_string_trim (paratext_line);
      int value = round (254 * convert_to_float (inches));
      float millimeters = (float) value / 10;
      style_definitions [paratext_marker] [leftmargin_key] = convert_to_string (millimeters);
      continue;
    }

    // Read and import the right margin.
    if (paratext_line.find (backslash_rightmargin) == 0) {
      paratext_line.erase (0, backslash_rightmargin.length());
      string inches = filter_string_trim (paratext_line);
      int value = round (254 * convert_to_float (inches));
      float millimeters = (float) value / 10;
      style_definitions [paratext_marker] [rightmargin_key] = convert_to_string (millimeters);
      continue;
    }

    // Read and import the first line indent.
    if (paratext_line.find (backslash_firstlineindent) == 0) {
      paratext_line.erase (0, backslash_firstlineindent.length());
      string inches = filter_string_trim (paratext_line);
      int value = round (254 * convert_to_float (inches));
      float millimeters = (float) value / 10;
      style_definitions [paratext_marker] [firstlineindent_key] = convert_to_string (millimeters);
      continue;
    }

    // Read and import the space before.
    if (paratext_line.find (backslash_spacebefore) == 0) {
      paratext_line.erase (0, backslash_spacebefore.length());
      string value = filter_string_trim (paratext_line);
      style_definitions [paratext_marker] [spacebefore_key] = value;
      continue;
    }

    // Read and import the space after.
    if (paratext_line.find (backslash_spaceafter) == 0) {
      paratext_line.erase (0, backslash_spaceafter.length());
      string value = filter_string_trim (paratext_line);
      style_definitions [paratext_marker] [spaceafter_key] = value;
      continue;
    }

  }
  

  // Build the C++ fragment with the default styles.
  cpp_lines.clear ();
  cpp_lines.push_back ("{");
  for (auto element : style_definitions) {
    definition_type style_definition = element.second;
    string line;
    cpp_lines.push_back ("  {");

    line = sources_style_parse_generate_entry (marker_key, style_definition [marker_key], true);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (name_key, style_definition [name_key], true);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (info_key, style_definition [info_key], true);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (category_key, style_definition [category_key], true);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (type_key, style_definition [type_key], false);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (subtype_key, style_definition [subtype_key], false);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (fontsize_key, style_definition [fontsize_key], false);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (italic_key, style_definition [italic_key], false);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (bold_key, style_definition [bold_key], false);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (underline_key, style_definition [underline_key], false);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (smallcaps_key, style_definition [smallcaps_key], false);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (superscript_key, style_definition [superscript_key], false);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (justification_key, style_definition [justification_key], false);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (spacebefore_key, style_definition [spacebefore_key], false);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (spaceafter_key, style_definition [spaceafter_key], false);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (leftmargin_key, style_definition [leftmargin_key], false);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (rightmargin_key, style_definition [rightmargin_key], false);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (firstlineindent_key, style_definition [firstlineindent_key], false);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (spancolumns_key, style_definition [spancolumns_key], false);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (color_key, style_definition [color_key], true);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (print_key, style_definition [print_key], false);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (userbool1_key, style_definition [userbool1_key], false);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (userbool2_key, style_definition [userbool2_key], false);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (userbool3_key, style_definition [userbool3_key], false);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (userint1_key, style_definition [userint1_key], false);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (userint2_key, style_definition [userint2_key], false);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (userint3_key, style_definition [userint3_key], false);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (userstring1_key, style_definition [userstring1_key], true);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (userstring2_key, style_definition [userstring2_key], true);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (userstring3_key, style_definition [userstring3_key], true);
    cpp_lines.push_back (line);
    line = sources_style_parse_generate_entry (backgroundcolor_key, style_definition [backgroundcolor_key], true);
    cpp_lines.push_back (line);

    cpp_lines.push_back ("  },");
  }
  
  
  // Insert the C++ fragment into the source code.
  contents = filter_url_file_get_contents (cpp_path);
  vector <string> source_lines = filter_string_explode (contents, '\n');
  vector <string> updated_lines;
  bool updating = false;
  for (auto source_line : source_lines) {
    if (updating) {
      if (source_line.find (cpp_end) != string::npos) updating = false;
    }
    if (!updating) {
      updated_lines.push_back (source_line);
    }
    if (source_line.find (cpp_start) != string::npos) {
      for (auto line : cpp_lines) {
        updated_lines.push_back (line);
      }
      updating = true;
    }
  }

  // Save it to the C++ source file.
  contents = filter_string_implode (updated_lines, "\n");
  filter_url_file_put_contents (cpp_path, contents);
  
  cout << "Finished parsing style values from the usfm.sty file" << endl;

}
