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


// Suppress errors in Visual Studio 2019.
// No longer needed since upgrading the UTF8 library?
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING 1
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING 1
#pragma GCC diagnostic ignored "-Wunused-macros"

#include <filter/string.h>
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wswitch-default"
#include <utf8/utf8.h>
#pragma GCC diagnostic pop
#include <filter/url.h>
#include <filter/md5.h>
#include <filter/date.h>
#include <database/config/general.h>
#include <database/logs.h>
#ifdef HAVE_UTF8PROC
#include <utf8proc.h>
#else
#include <utf8proc/utf8proc.h>
#endif
#include <config/globals.h>
#ifdef HAVE_WINDOWS
#include <codecvt>
#endif
#ifdef HAVE_ICU
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Wsign-conversion"
#include <unicode/ustdio.h>
#include <unicode/normlzr.h>
#include <unicode/utypes.h>
#include <unicode/unistr.h>
#include <unicode/translit.h>
#pragma clang diagnostic pop
#endif
#ifdef HAVE_CLOUD
#include <libxml/tree.h>
#include <libxml/HTMLparser.h>
#endif
#ifdef HAVE_CLOUD
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <gumbo.h>
#pragma clang diagnostic pop
#endif
#ifdef HAVE_CLOUD
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
//#include <tidy.h>
//#include <tidybuffio.h>
#include "tidy/tidy.h"
#include "tidy/tidybuffio.h"
#pragma clang diagnostic pop
#endif
#include <stdio.h>
#include <errno.h>
using namespace std;
#ifdef HAVE_ICU
using namespace icu;
#endif


// A C++ equivalent for PHP's explode function.
// Split a string on a delimiter.
// Return a vector of strings.
std::vector <string> filter_string_explode (string value, char delimiter)
{
  std::vector <string> result;
  istringstream iss (value);
  for (string token; getline (iss, token, delimiter); )
  {
    result.push_back (move (token));
  }
  return result;
}


// Explodes an input string on multiple delimiters.
vector <string> filter_string_explode (string value, string delimiters)
{
  vector <string> result;
  while (!value.empty ()) {
    size_t pos = value.find_first_of (delimiters);
    if (pos == string::npos) {
      result.push_back (value);
      value.clear ();
    } else {
      string s = value.substr (0, pos);
      if (!s.empty()) result.push_back (s);
      pos++;
      value.erase (0, pos);
    }
  }
  return result;
}


// A C++ equivalent for PHP's implode function.
// Join a vector of string, with delimiters, into a string.
// Return this string.
string filter_string_implode (vector <string>& values, string delimiter)
{
  string full;
  for (vector<string>::iterator it = values.begin (); it != values.end (); ++it)
  {
    full += (*it);
    if (it != values.end ()-1) full += delimiter;
  }
  return full;  
}


// A C++ rough equivalent for PHP's filter_string_str_replace function.
string filter_string_str_replace (string search, string replace, string subject, int * count)
{
  size_t offposition = subject.find (search);
  while (offposition != string::npos) {
    subject.replace (offposition, search.length (), replace);
    if (count) (*count)++;
    offposition = subject.find (search, offposition + replace.length ());
  }
  return subject;
}


// Replaces text that starts with "start" and ends with "end" with "replacement".
// Returns true if replacement was done.
bool filter_string_replace_between (string& line, const string& start, const string& end, const string& replacement)
{
  bool replacements_done = false;
  size_t beginpos = line.find (start);
  size_t endpos = line.find (end);
  while ((beginpos != string::npos) && (endpos != string::npos) && (endpos > beginpos)) {
    line.replace (beginpos, endpos - beginpos + end.length (), replacement);
    beginpos = line.find (start, beginpos + replacement.length ());
    endpos = line.find (end, beginpos + replacement.length ());
    replacements_done = true;
  }
  return replacements_done;
}


// It replaces a copy of string delimited by the start and length parameters with the string given in replacement.
// It is similar to PHP's function with the same name.
string substr_replace (string original, string replacement, size_t start, size_t length)
{
  if (length) original.erase (start, length);
  original.insert (start, replacement);
  return original;
}



// On some platforms the sizeof (unsigned int) is equal to the sizeof (size_t).
// Then compilation would fail if there were two functions "convert_to_string",
// one taking the unsigned int, and the other taking the size_t.
// Therefore there is now one function doing both.
// This may lead to embiguity errors for the C++ compiler.
// In such case the ambiguity can be removed by changing the type to be passed
// to this function to "size_t".
string convert_to_string (size_t i)
{
  ostringstream r;
  r << i;
  return r.str();
}


string convert_to_string (int i)
{
  ostringstream r;
  r << i;
  return r.str();
}


string convert_to_string (char * c)
{
  string s = c;
  return s;
}


string convert_to_string (const char * c)
{
  string s = c;
  return s;
}


string convert_to_string (bool b)
{
  if (b) return "1";
  return "0";
}


string convert_to_string (string s)
{
  return s;
}


string convert_to_string (float f)
{
  ostringstream r;
  r << f;
  return r.str();
}


int convert_to_int (string s)
{
  int i = atoi (s.c_str());
  return i;
}


int convert_to_int (float f)
{
  int i = static_cast<int> (round(f));
  return i;
}


long long convert_to_long_long (string s)
{
  long long i = 0;
  istringstream r (s);
  r >> i;
  return i;
}


float convert_to_float (string s)
{
  float f = 0;
  istringstream r (s);
  r >> f;
  return f;
}


bool convert_to_bool (string s)
{
  if (s.empty()) return false;
  if (s == "true") return true;
  if (s == "TRUE") return true;
  bool b;
  istringstream (s) >> b;
  return b;
}


string convert_to_true_false (bool b)
{
  if (b) return "true";
  return "false";
}


u16string convert_to_u16string (string s)
{
  wstring_convert <codecvt_utf8_utf16 <char16_t>, char16_t> utf16conv;
  u16string utf16 = utf16conv.from_bytes (s);
  // utf16.length()
  return utf16;
}


// A C++ equivalent for PHP's array_unique function.
vector <string> array_unique (vector <string> values)
{
  vector <string> result;
  set <string> unique;
  for (unsigned int i = 0; i < values.size (); i++) {
    if (unique.find (values[i]) == unique.end ()) {
      unique.insert (values[i]);
      result.push_back ((values[i]));
    }
  }
  return result;
}


// A C++ equivalent for PHP's array_unique function.
vector <int> array_unique (vector <int> values)
{
  vector <int> result;
  set <int> unique;
  for (unsigned int i = 0; i < values.size (); i++) {
    if (unique.find (values[i]) == unique.end ()) {
      unique.insert (values[i]);
      result.push_back ((values[i]));
    }
  }
  return result;
}


// A C++ equivalent for PHP's array_diff function.
// Returns items in "from" which are not present in "against".
vector <string> filter_string_array_diff (vector <string> from, vector <string> against)
{
  vector <string> result;
  set <string> against2 (against.begin (), against.end ());
  for (unsigned int i = 0; i < from.size (); i++) {
    if (against2.find (from[i]) == against2.end ()) {
      result.push_back ((from[i]));
    }
  }
  return result;
}


// A C++ equivalent for PHP's array_diff function.
// Returns items in "from" which are not present in "against".
vector <int> filter_string_array_diff (vector <int> from, vector <int> against)
{
  vector <int> result;
  set <int> against2 (against.begin (), against.end ());
  for (unsigned int i = 0; i < from.size (); i++) {
    if (against2.find (from[i]) == against2.end ()) {
      result.push_back ((from[i]));
    }
  }
  return result;
}


// A C++ equivalent for PHP's filter_string_trim function.
string filter_string_trim (string s)
{
  if (s.length () == 0)
    return s;
  // Strip spaces, tabs, new lines, carriage returns.
  size_t beg = s.find_first_not_of(" \t\n\r");
  size_t end = s.find_last_not_of(" \t\n\r");
  // No non-spaces  
  if (beg == string::npos)
    return "";
  return string (s, beg, end - beg + 1);
}


// A C++ equivalent for PHP's filter_string_ltrim function.
string filter_string_ltrim (string s)
{
  if (s.length () == 0) return s;
  // Strip spaces, tabs, new lines, carriage returns.
  size_t pos = s.find_first_not_of(" \t\n\r");
  // No non-spaces  
  if (pos == string::npos) return "";
  return s.substr (pos);
}


// Right trim string.
string filter_string_rtrim (string s)
{
  if (s.length () == 0) return s;
  // Strip spaces, tabs, new lines, carriage returns.
  size_t pos = s.find_last_not_of(" \t\n\r");
  // No non-spaces
  if (pos == string::npos) return string();
  // Erase it.
  s.erase (pos + 1);
  // Done.
  return s;
}


// Fills a string up to "width", with the character "fill" at the left.
string filter_string_fill (string s, int width, char fill)
{
  ostringstream str;
  str << setfill (fill) << setw (width) << s;
  return str.str();
}


// Returns true/false whether s is numeric.
bool filter_string_is_numeric (string s)
{
  for (char c : s) if (!isdigit (c)) return false;
  return true;
}


// This takes the five special XML characters and escapes them.
// < : &lt;
// & : &amp;
// > : &gt;
// " : &quot;
// ' : &apos;
string escape_special_xml_characters (string s)
{
  s = filter_string_str_replace ("&", "&amp;", s);
  s = filter_string_str_replace (R"(")", "&quot;", s);
  s = filter_string_str_replace ("'", "&apos;", s);
  s = filter_string_str_replace ("<", "&lt;", s);
  s = filter_string_str_replace (">", "&gt;", s);
  return s;
}


// This unescapes the five special XML characters.
string unescape_special_xml_characters (string s)
{
  s = filter_string_str_replace ("&quot;", R"(")", s);
  s = filter_string_str_replace ("&amp;", "&", s);
  s = filter_string_str_replace ("&apos;", "'", s);
  s = filter_string_str_replace ("&lt;", "<", s);
  s = filter_string_str_replace ("&gt;", ">", s);
  return s;
}


// Converts other types of spaces to standard spaces.
string any_space_to_standard_space (string s)
{
  s = filter_string_str_replace (unicode_non_breaking_space_entity (), " ", s);
  s = filter_string_str_replace (non_breaking_space_u00A0 (), " ", s);
  s = filter_string_str_replace (en_space_u2002 (), " ", s);
  s = filter_string_str_replace (figure_space_u2007 (), " ", s);
  s = filter_string_str_replace (narrow_non_breaking_space_u202F (), " ", s);
  return s;
}


// Returns a no-break space (NBSP) (x00A0).
string non_breaking_space_u00A0 ()
{
#ifdef HAVE_WINDOWS
  // On Visual Studio 2015 the C-style code below does not work.
  return " ";
#endif
  // Use C-style code.
  return "\u00A0";
}


// Returns a soft hyphen.
string soft_hyphen_u00AD ()
{
#ifdef HAVE_WINDOWS
  // On Visual Studio 2015 the C-style code below does not work.
  return "­";
#endif
  // Soft hyphen U+00AD.
  return "\u00AD";
}


// Returns an "en space", this is a nut, half an em space.
string en_space_u2002 ()
{
#ifdef HAVE_WINDOWS
  // On Visual Studio 2015 the C-style code below does not work.
  return " ";
#endif
  // U+2002.
  return "\u2002";
}


// A "figure space".
// A space to use in numbers.
// It has the same width as the digits.
// It does not break, it keeps the numbers together.
string figure_space_u2007 ()
{
#ifdef HAVE_WINDOWS
  // On Visual Studio 2015 the C-style code below does not work.
  return " ";
#endif
  return "\u2007";
}


// Returns a "narrow no-break space (x202F)
string narrow_non_breaking_space_u202F ()
{
#ifdef HAVE_WINDOWS
  // On Visual Studio 2015 the C-style code below does not work.
  return " ";
#endif
  return "\u202F";
}


// Returns the length of string s in unicode points, not in bytes.
size_t unicode_string_length (string s)
{
  size_t length = static_cast<size_t> (utf8::distance (s.begin(), s.end()));
  return length;
}


// Get the substring with unicode point pos(ition) and len(gth).
// If len = 0, the string from start till end is returned.
string unicode_string_substr (string s, size_t pos, size_t len)
{
  char * input = const_cast<char *>(s.c_str());
  char * startiter = input;
  size_t length = strlen (input);
  char * veryend = input + length + 1;
  // Iterate forward pos times.
  while (pos > 0) {
    if (strlen (startiter)) {
      utf8::next (startiter, veryend);
    } else {
      // End reached: Return empty result.
      return "";
    }
    pos--;
  }
  // Zero len: Return result till the end of the string.
  if (len == 0) {
    s.assign (startiter);
    return s;
  }

  // Iterate forward len times.
  char * enditer = startiter;
  while (len > 0) {
    if (strlen (enditer)) {
      utf8::next (enditer, veryend);
    } else {
      // End reached: Return result.
      s.assign (startiter);
      return s;
    }
    len--;
  }
  // Return substring.
  size_t startpos = static_cast <size_t> (startiter - input);
  size_t lenpos = static_cast <size_t> (enditer - startiter);
  s = s.substr (startpos, lenpos);
  return s;
}


// Equivalent to PHP's mb_strpos function.
size_t unicode_string_strpos (string haystack, string needle, size_t offset)
{
  int haystack_length = static_cast<int>(unicode_string_length (haystack));
  int needle_length = static_cast<int>(unicode_string_length (needle));
  for (int pos = static_cast<int>(offset); pos <= haystack_length - needle_length; pos++) {
    string substring = unicode_string_substr (haystack, static_cast <size_t> (pos), static_cast <size_t> (needle_length));
    if (substring == needle) return static_cast <size_t> (pos);
  }
  return string::npos;
}


// Case-insensitive version of "unicode_string_strpos".
size_t unicode_string_strpos_case_insensitive (string haystack, string needle, size_t offset)
{
  haystack = unicode_string_casefold (haystack);
  needle = unicode_string_casefold (needle);
  
  int haystack_length = static_cast<int>(unicode_string_length (haystack));
  int needle_length = static_cast<int>(unicode_string_length (needle));
  for (int pos = static_cast<int>(offset); pos <= haystack_length - needle_length; pos++) {
    string substring = unicode_string_substr (haystack, static_cast<size_t> (pos), static_cast<size_t> (needle_length));
    if (substring == needle) return static_cast<size_t>(pos);
  }
  return string::npos;
}


// Converts string to lowercase.
string unicode_string_casefold (string s)
{
  string casefolded;
  // The conversion routine below is slow.
  // There was a case that a user tried to put a whole Bible into one chapter.
  // As a result, the Cloud choked on converting this chapter to lower case.
  // So measurements were done as to the time the routine below takes to convert data.
  // Processor 1,2 GHz Intel Core m3 took 1.5 minutes to convert 35 kbytes of data.
  // So a limit had to be introduced below,
  // that is the input text is larger than this limit,
  // the routine won't fold it.
  if (s.size () > 35000) {
    return s;
  }
  // Do the case folding.
  try {
    // The UTF8 processor works with one Unicode point at a time.
    size_t string_length = unicode_string_length (s);
    for (unsigned int pos = 0; pos < string_length; pos++) {
      // Get one UTF-8 character.
      string character = unicode_string_substr (s, pos, 1);
      // Convert it to a Unicode point.
      const utf8proc_uint8_t *str = reinterpret_cast<const unsigned char *> (character.c_str ());
      utf8proc_ssize_t len = static_cast<utf8proc_ssize_t> (character.length ());
      utf8proc_int32_t dst;
      [[maybe_unused]] utf8proc_ssize_t output = utf8proc_iterate (str, len, &dst);
      // Convert the Unicode point to lower case.
      utf8proc_int32_t luc = utf8proc_tolower (dst);
      // Convert the Unicode point back to a UTF-8 string.
      utf8proc_uint8_t buffer [10];
      output = utf8proc_encode_char (luc, buffer);
      buffer [output] = 0;
      stringstream ss;
      ss << buffer;
      // Add the casefolded UTF-8 character to the result.
      casefolded.append (ss.str ());
    }
  } catch (...) {
  }
  // Done.
  return casefolded;
/*
 The code below shows how to do it through the ICU library.
 But the ICU library could not be compiled properly for Android.
 Therefore it is not used on any platform.

  // UTF-8 string -> UTF-16 UnicodeString
  UnicodeString source = UnicodeString::fromUTF8 (StringPiece (s));
  // Case folding.
  source.foldCase ();
  // UTF-16 UnicodeString -> UTF-8 std::string
  string result;
  source.toUTF8String (result);
  // Ready.
  return result;
*/
}


string unicode_string_uppercase (string s)
{
  string uppercase;
  try {
    // The UTF8 processor works with one Unicode point at a time.
    size_t string_length = unicode_string_length (s);
    for (unsigned int pos = 0; pos < string_length; pos++) {
      // Get one UTF-8 character.
      string character = unicode_string_substr (s, pos, 1);
      // Convert it to a Unicode point.
      const utf8proc_uint8_t *str = reinterpret_cast<const unsigned char *> (character.c_str ());
      utf8proc_ssize_t len = static_cast<utf8proc_ssize_t> (character.length ());
      utf8proc_int32_t dst;
      [[maybe_unused]] utf8proc_ssize_t output = utf8proc_iterate (str, len, &dst);
      // Convert the Unicode point to lower case.
      utf8proc_int32_t luc = utf8proc_toupper (dst);
      // Convert the Unicode point back to a UTF-8 string.
      utf8proc_uint8_t buffer [10];
      output = utf8proc_encode_char (luc, buffer);
      buffer [output] = 0;
      stringstream ss;
      ss << buffer;
      // Add the casefolded UTF-8 character to the result.
      uppercase.append (ss.str ());
    }
  } catch (...) {
  }
  // Done.
  return uppercase;
/*
 How to do the above through the ICU library.
  UnicodeString source = UnicodeString::fromUTF8 (StringPiece (s));
  source.toUpper ();
  string result;
  source.toUTF8String (result);
  return result;
*/
}


string unicode_string_transliterate (string s)
{
  string transliteration;
  try {
    size_t string_length = unicode_string_length (s);
    for (unsigned int pos = 0; pos < string_length; pos++) {
      string character = unicode_string_substr (s, pos, 1);
      const utf8proc_uint8_t *str = reinterpret_cast<const unsigned char *> (character.c_str ());
      utf8proc_ssize_t len = static_cast<utf8proc_ssize_t> (character.length ());
      uint8_t *dest;
      utf8proc_option_t options = static_cast<utf8proc_option_t> (UTF8PROC_DECOMPOSE | UTF8PROC_STRIPMARK);
      [[maybe_unused]] auto output = utf8proc_map (str, len, &dest, options);
      stringstream ss;
      ss << dest;
      transliteration.append (ss.str ());
      free (dest);
    }
  } catch (...) {
  }
  return transliteration;
/*
 Code showing how to do the transliteration through the ICU library.
  // UTF-8 string -> UTF-16 UnicodeString
  UnicodeString source = UnicodeString::fromUTF8 (StringPiece (s));
  
  // Transliterate UTF-16 UnicodeString following this rule:
  // decompose, remove diacritics, recompose
  UErrorCode status = U_ZERO_ERROR;
  Transliterator *transliterator = Transliterator::createInstance("NFD; [:M:] Remove; NFC",
                                                                  UTRANS_FORWARD,
                                                                  status);
  transliterator->transliterate(source);
  
  // UTF-16 UnicodeString -> UTF-8 std::string
  string result;
  source.toUTF8String (result);
  
  // Done.
  return result;
 */
}


// Returns true if string "s" is valid UTF8 encoded.
bool unicode_string_is_valid (string s)
{
  return utf8::is_valid (s.begin(), s.end());
}


// Returns whether $s is Unicode punctuation.
bool unicode_string_is_punctuation (string s)
{
  try {
    if (s.empty ()) return false;
    // Be sure to take only one character.
    s = unicode_string_substr (s, 0, 1);
    // Convert the string to a Unicode point.
    const utf8proc_uint8_t *str = reinterpret_cast<const unsigned char *>(s.c_str ());
    utf8proc_ssize_t len = static_cast<utf8proc_ssize_t> (s.length ());
    utf8proc_int32_t codepoint;
    [[maybe_unused]] auto output = utf8proc_iterate (str, len, &codepoint);
    // Get category.
    utf8proc_category_t category = utf8proc_category	(codepoint);
    if ((category >= UTF8PROC_CATEGORY_PC) && (category <= UTF8PROC_CATEGORY_PO)) return true;
  } catch (...) {
  }
  return false;
  /* 
  The following code shows how to do the above code through the ICU library.
  UnicodeString source = UnicodeString::fromUTF8 (StringPiece (s));
  StringCharacterIterator iter (source);
  UChar32 character = iter.first32 ();
  bool punctuation = u_ispunct (character);
  return punctuation;
  */
}


// Converts the string $s to a Unicode codepoint.
int unicode_string_convert_to_codepoint (string s)
{
  int point = 0;
  if (!s.empty ()) {
    try {
      // Be sure to take only one character.
      s = unicode_string_substr (s, 0, 1);
      // Convert the string to a Unicode point.
      const utf8proc_uint8_t *str = reinterpret_cast<const unsigned char *>(s.c_str ());
      utf8proc_ssize_t len = static_cast<utf8proc_ssize_t> (s.length ());
      utf8proc_int32_t codepoint;
      [[maybe_unused]] auto output = utf8proc_iterate (str, len, &codepoint);
      point = codepoint;
    } catch (...) {
    }
    
  }
  return point;
}


string unicode_string_str_replace (string search, string replace, string subject)
{
  // The needle to look for should not be empty.
  if (!search.empty ()) {
    // Do the replacing.
    size_t searchlength = unicode_string_length (search);
    size_t offposition = unicode_string_strpos (subject, search);
    while (offposition != string::npos) {
      string subject_before;
      // Due to the nature of the substr finder, it needs special handling for search position zero.
      if (offposition != 0) subject_before = unicode_string_substr (subject, 0, offposition);
      // Continue with the splitting and joining.
      string subject_after = unicode_string_substr (subject, offposition + searchlength, subject.length());
      subject = subject_before + replace + subject_after;
      // Prepare for next iteration.
      offposition = unicode_string_strpos (subject, search, offposition + unicode_string_length (replace));
    }
  }
  // Ready.
  return subject;
}


#ifdef HAVE_ICU
string icu_string_normalize (string s, bool remove_diacritics, bool casefold)
{
  // Skip any conversions that slow things down if no normalization is to be done.
  if (!remove_diacritics && !casefold) return s;
  
  // UTF-8 std::string -> UTF-16 UnicodeString
  UnicodeString source = UnicodeString::fromUTF8 (StringPiece (s));

  // The order of doing the normalization action may be of influence on the result.
  // Right now it seems more logical to remove diacritics first and then doing the case folding.

  // Removal of diacritics.
  if (remove_diacritics) {
    // Transliterate UTF-16 UnicodeString following this rule:
    // decompose, remove diacritics, recompose
    UErrorCode status = U_ZERO_ERROR;
    Transliterator *accents_converter = Transliterator::createInstance("NFD; [:M:] Remove; NFC", UTRANS_FORWARD, status);
    accents_converter->transliterate(source);
  }

  // Case folding.
  if (casefold) {
    source.foldCase ();
  }
  
  // UTF-16 UnicodeString -> UTF-8 std::string
  string result;
  source.toUTF8String (result);
  
  return result;
}
#endif


// Some code for when it's necessary to find out if text is alphabetic.
//#include <unicode/uchar.h>
//#include <unicode/unistr.h>
//#include <unicode/schriter.h>
//const std::string& word = static_cast<const std::string&>(symbol);
//icu::UnicodeString uword = icu::UnicodeString::fromUTF8(icu::StringPiece(word.data(), word.size()));
//
//std::string signature = "<unk";
//
//// signature for English, taken from Stanford parser's getSignature5
//int num_caps = 0;
//bool has_digit  = false;
//bool has_dash   = false;
//bool has_lower  = false;
//bool has_punct  = false;
//bool has_symbol = false;
//
//size_t length = 0;
//UChar32 ch0 = 0;
//UChar32 ch_1 = 0;
//UChar32 ch_2 = 0;
//
//icu::StringCharacterIterator iter(uword);
//for (iter.setToStart(); iter.hasNext(); ++ length) {
//  const UChar32 ch = iter.next32PostInc();
//
//  // keep initial char...
//  if (ch0 == 0)
//    ch0 = ch;
//
//  ch_2 = ch_1;
//  ch_1 = ch;
//
//  const int32_t gc = u_getIntPropertyValue(ch, UCHAR_GENERAL_CATEGORY_MASK);
//
//  has_dash   |= ((gc & U_GC_PD_MASK) != 0);
//  has_punct  |= ((gc & U_GC_P_MASK) != 0);
//  has_symbol |= ((gc & U_GC_S_MASK) != 0);
//
//  has_digit  |= (u_getNumericValue(ch) != U_NO_NUMERIC_VALUE);
//
//  if (u_isUAlphabetic(ch)) {
//    if (u_isULowercase(ch))
//      has_lower = true;
//    else if (u_istitle(ch)) {
//      has_lower = true;
//      ++ num_caps;
//    } else
//      ++ num_caps;
//  }
//}
//
//// transform into lower...
//uword.toLower();
//ch_2 = (ch_2 ? u_tolower(ch_2) : ch_2);
//ch_1 = (ch_1 ? u_tolower(ch_1) : ch_1);
//
//// we do not check loc...
//if (u_isUUppercase(ch0) || u_istitle(ch0))
//  signature += "-caps";
//else if (! u_isUAlphabetic(ch0) && num_caps)
//  signature += "-caps";
//else if (has_lower)
//  signature += "-lc";
//
//if (has_digit)
//  signature += "-num";
//if (has_dash)
//  signature += "-dash";
//if (has_punct)
//  signature += "-punct";
//if (has_symbol)
//  signature += "-sym";
//
//if (length >= 3 && ch_1 == 's') {
//  if (ch_2 != 's' && ch_2 != 'i' && ch_2 != 'u')
//    signature += "-s";
//} else if (length >= 5 && ! has_dash && ! (has_digit && num_caps > 0)) {
//  if (uword.endsWith("ed"))
//    signature += "-ed";
//  else if (uword.endsWith("ing"))
//    signature += "-ing";
//  else if (uword.endsWith("ion"))
//    signature += "-ion";
//  else if (uword.endsWith("er"))
//    signature += "-er";
//  else if (uword.endsWith("est"))
//    signature += "-est";
//  else if (uword.endsWith("ly"))
//    signature += "-ly";
//  else if (uword.endsWith("ity"))
//    signature += "-ity";
//  else if (uword.endsWith("y"))
//    signature += "-y";
//  else if (uword.endsWith("al"))
//    signature += "-al";
//}


// Generate a truly random number between $floor and $ceiling.
int filter_string_rand (int floor, int ceiling)
{
  int range = ceiling - floor;
  int r = config_globals_int_distribution (config_globals_random_engine) % range + floor;
  return r;
}


string filter_string_html2text (string html)
{
  // Clean the html up.
  html = filter_string_str_replace ("\n", string(), html);

  // The output text.
  string text {};

  // Keep going while the html contains the < character.
  size_t pos {html.find ("<")};
  while (pos != string::npos) {
    // Add the text before the <.
    text.append (html.substr (0, pos));
    html = html.substr (pos + 1);
    // Certain tags start new lines.
    string tag1 {unicode_string_casefold (html.substr (0, 1))};
    string tag2 {unicode_string_casefold (html.substr (0, 2))};
    string tag3 {unicode_string_casefold (html.substr (0, 3))};
    if  ((tag1 == "p")
      || (tag3 == "div")
      || (tag2 == "li")
      || (tag3 == "/ol")
      || (tag3 == "/ul")
      || (tag2 == "h1")
      || (tag2 == "h2")
      || (tag2 == "h3")
      || (tag2 == "h4")
      || (tag2 == "h5")
      || (tag2 == "br")
       ) {
      text.append ("\n");
    }
    // Clear text out till the > character.
    pos = html.find (">");
    if (pos != string::npos) {
      html = html.substr (pos + 1);
    }
    // Next iteration.
    pos = html.find ("<");
  }
  // Add any remaining bit of text.
  text.append (html);

  // Replace xml entities with their text.
  text = unescape_special_xml_characters (text);

  while (text.find ("\n\n") != string::npos) {
    text = filter_string_str_replace ("\n\n", "\n", text);
  }
  text = filter_string_trim (text);
  return text;
}


// Extracts the pure email address from a string.
// input: Foo Bar <foo@bar.nl>
// input: foo@bar.nl
// Returns: foo@bar.nl
// If there is no valid email, it returns false.
string filter_string_extract_email (string input)
{
  size_t pos = input.find ("<");
  if (pos != string::npos) {
    input = input.substr (pos + 1);
  }
  pos = input.find (">");
  if (pos != string::npos) {
    input = input.substr (0, pos);
  }
  string email = input;
  if (!filter_url_email_is_valid (email)) email.clear();
  return email;
}


// Extracts a clean string from the email body given in input.
// It leaves out the bit that was quoted.
// If year and sender are given, it also removes lines that contain both strings.
// This is used to remove lines like:
// On Wed, 2011-03-02 at 08:26 +0100, Bibledit wrote:
string filter_string_extract_body (string input, string year, string sender)
{
  vector <string> inputlines = filter_string_explode (input, '\n');
  if (inputlines.empty ()) return "";
  vector <string> body;
  for (string & line : inputlines) {
    string trimmed = filter_string_trim (line);
    if (trimmed == "") continue;
    if (trimmed.find (">") == 0) continue;
    if ((year != "") && (sender != "")) {
      if (trimmed.find (year) != string::npos) {
        if (trimmed.find (sender) != string::npos) {
          continue;
        }
      }
    }
    body.push_back (line);
  }
  string bodystring = filter_string_implode (body, "\n");
  bodystring = filter_string_trim (bodystring);
  return bodystring;
}


// Returns an appropriate value.
string get_checkbox_status (bool enabled)
{
  if (enabled) return "checked";
  return "";
}


string get_disabled (bool disabled)
{
  if (disabled) return "disabled";
  return string();
}


string get_reload ()
{
  return string("reload");
}


void quick_swap(string & a, string & b)
{
  string t = a;
  a = b;
  b = t;
}


void quick_swap(unsigned int &a, unsigned int &b)
{
  unsigned int t = a;
  a = b;
  b = t;
}


void quick_swap(long unsigned int &a, long unsigned int &b)
{
  long unsigned int t = a;
  a = b;
  b = t;
}


void quick_swap(int &a, int &b)
{
  int t = a;
  a = b;
  b = t;
}


void quick_swap(bool & a, bool & b)
{
  bool t = a;
  a = b;
  b = t;
}


// This function is unusual in the sense that it does not sort one container,
// as the majority of sort functions do, but it accepts two containers.
// It sorts on the first, and reorders the second container at the same time,
// following the reordering done in the first container.
void quick_sort (vector <unsigned int>& one, vector <string> &two, unsigned int beg, unsigned int end)
{
  if (end > beg + 1) {
    unsigned int piv = one[beg];
    unsigned int l = beg + 1;
    unsigned int r = end;
    while (l < r) {
      if (one[l] <= piv) {
        l++;
      } else {
        --r;
        quick_swap(one[l], one[r]);
        quick_swap(two[l], two[r]);
      }
    }
    --l;
    quick_swap(one[l], one[beg]);
    quick_swap(two[l], two[beg]);
    quick_sort(one, two, beg, l);
    quick_sort(one, two, r, end);
  }
}


void quick_sort(vector < string > &one, vector < unsigned int >&two, unsigned int beg, unsigned int end)
{
  if (end > beg + 1) {
    string piv = one[beg];
    unsigned int l = beg + 1;
    unsigned int r = end;
    while (l < r) {
      if (one[l] <= piv) {
        l++;
      } else {
        --r;
        quick_swap(one[l], one[r]);
        quick_swap(two[l], two[r]);
      }
    }
    --l;
    quick_swap(one[l], one[beg]);
    quick_swap(two[l], two[beg]);
    quick_sort(one, two, beg, l);
    quick_sort(one, two, r, end);
  }
}


void quick_sort(vector < unsigned int >&one, vector < unsigned int >&two, unsigned int beg, unsigned int end)
{
  if (end > beg + 1) {
    unsigned int piv = one[beg];
    unsigned int l = beg + 1;
    unsigned int r = end;
    while (l < r) {
      if (one[l] <= piv) {
        l++;
      } else {
        --r;
        quick_swap(one[l], one[r]);
        quick_swap(two[l], two[r]);
      }
    }
    --l;
    quick_swap(one[l], one[beg]);
    quick_swap(two[l], two[beg]);
    quick_sort(one, two, beg, l);
    quick_sort(one, two, r, end);
  }
}


void quick_sort (vector<unsigned int>& one, vector<bool>& two, unsigned int beg, unsigned int end)
{
  if (end > beg + 1) {
    unsigned int piv = one[beg];
    unsigned int l = beg + 1;
    unsigned int r = end;
    while (l < r) {
      if (one[l] <= piv) {
        l++;
      } else {
        --r;
        quick_swap(one[l], one[r]);
        bool two_l = two[l];
        bool two_r = two[r];
        quick_swap(two_l, two_r);
        two[l] = two_l;
        two[r] = two_r;
      }
    }
    --l;
    quick_swap(one[l], one[beg]);
    bool two_l = two[l];
    bool two_beg = two[beg];
    quick_swap(two_l, two_beg);
    two[l] = two_l;
    two[beg] = two_beg;
    quick_sort(one, two, beg, l);
    quick_sort(one, two, r, end);
  }
}


void quick_sort(vector < int >&one, vector < unsigned int >&two, unsigned int beg, unsigned int end)
{
  if (end > beg + 1) {
    int piv = one[beg];
    unsigned int l = beg + 1;
    unsigned int r = end;
    while (l < r) {
      if (one[l] <= piv) {
        l++;
      } else {
        --r;
        quick_swap(one[l], one[r]);
        quick_swap(two[l], two[r]);
      }
    }
    --l;
    quick_swap(one[l], one[beg]);
    quick_swap(two[l], two[beg]);
    quick_sort(one, two, beg, l);
    quick_sort(one, two, r, end);
  }
}

void quick_sort(vector < string > &one, vector < string > &two, unsigned int beg, unsigned int end)
{
  if (end > beg + 1) {
    string piv = one[beg];
    unsigned int l = beg + 1;
    unsigned int r = end;
    while (l < r) {
      if (one[l] <= piv) {
        l++;
      } else {
        --r;
        quick_swap(one[l], one[r]);
        quick_swap(two[l], two[r]);
      }
    }
    --l;
    quick_swap(one[l], one[beg]);
    quick_swap(two[l], two[beg]);
    quick_sort(one, two, beg, l);
    quick_sort(one, two, r, end);
  }
}


void quick_sort(vector < string > &one, vector < bool > &two, unsigned int beg, unsigned int end)
{
  if (end > beg + 1) {
    string piv = one[beg];
    unsigned int l = beg + 1;
    unsigned int r = end;
    while (l < r) {
      if (one[l] <= piv) {
        l++;
      } else {
        --r;
        quick_swap(one[l], one[r]);
        bool two_l = two[l];
        bool two_r = two[r];
        quick_swap(two_l, two_r);
        two[l] = two_l;
        two[r] = two_r;
      }
    }
    --l;
    quick_swap(one[l], one[beg]);
    bool two_l = two[l];
    bool two_beg = two[beg];
    quick_swap(two_l, two_beg);
    two[l] = two_l;
    two[beg] = two_beg;
    quick_sort(one, two, beg, l);
    quick_sort(one, two, r, end);
  }
}


void quick_sort(vector < string > &one, unsigned int beg, unsigned int end)
{
  if (end > beg + 1) {
    string piv = one[beg];
    unsigned int l = beg + 1;
    unsigned int r = end;
    while (l < r) {
      if (one[l] <= piv) {
        l++;
      } else {
        --r;
        quick_swap(one[l], one[r]);
      }
    }
    --l;
    quick_swap(one[l], one[beg]);
    quick_sort(one, beg, l);
    quick_sort(one, r, end);
  }
}


void quick_sort(vector <long unsigned int>& one, vector <long unsigned int>& two, unsigned int beg, unsigned int end)
{
  if (end > beg + 1) {
    long unsigned int piv = one[beg];
    unsigned int l = beg + 1;
    unsigned int r = end;
    while (l < r) {
      if (one[l] <= piv) {
        l++;
      } else {
        --r;
        quick_swap(one[l], one[r]);
        quick_swap(two[l], two[r]);
      }
    }
    --l;
    quick_swap(one[l], one[beg]);
    quick_swap(two[l], two[beg]);
    quick_sort(one, two, beg, l);
    quick_sort(one, two, r, end);
  }
}


void quick_sort (vector <int> & one, vector <int> & two, unsigned int beg, unsigned int end)
{
  if (end > beg + 1) {
    int piv = one[beg];
    unsigned int l = beg + 1;
    unsigned int r = end;
    while (l < r) {
      if (one[l] <= piv) {
        l++;
      } else {
        --r;
        quick_swap(one[l], one[r]);
        quick_swap(two[l], two[r]);
      }
    }
    --l;
    quick_swap(one[l], one[beg]);
    quick_swap(two[l], two[beg]);
    quick_sort(one, two, beg, l);
    quick_sort(one, two, r, end);
  }
}


#define MY_NUMBERS "0123456789"
string number_in_string (const string & str)
{
  // Looks for and returns a positive number in a string.
  string output = str;
  output.erase (0, output.find_first_of (MY_NUMBERS));
  size_t end_position = output.find_first_not_of (MY_NUMBERS);
  if (end_position != string::npos) {
    output.erase (end_position, output.length());
  }
  return output;
}
#undef MY_NUMBERS



// This function marks the array of $words in the string $text.
// It uses the <mark> markup for display as html.
string filter_string_markup_words (const vector <string>& words, string text)
{
  // Array of needles to look for.
  // The needles contain the search $words as they occur in the $text
  // in upper case or lower case, or any mixed case.
  vector <string> needles;
  for (auto & word : words) {
    if (word == "") continue;
    vector <string> new_needles = filter_string_search_needles (word, text);
    needles.insert (needles.end(), new_needles.begin(), new_needles.end());
  }
  needles = array_unique (needles);
  
  // All the $needles are converted to $markup,
  // which will replace the $needles.
  for (auto & needle : needles) {
    string markup = "<mark>" + needle + "</mark>";
    text = filter_string_str_replace (needle, markup, text);
  }
  
  // Result.
  return text;
}


// This function returns an array of needles to look for.
// The needles contain the $search word as it occurs in the $string
// in upper case or lower case or any mixed case.
vector <string> filter_string_search_needles (string search, string text)
{
  vector <string> needles;
  size_t position = unicode_string_strpos_case_insensitive (text, search, 0);
  while (position != string::npos) {
    string needle = unicode_string_substr (text, position, unicode_string_length (search));
    needles.push_back (needle);
    position = unicode_string_strpos_case_insensitive (text, search, position + 1);
  }
  needles = array_unique (needles);
  return needles;
}


// Returns an integer identifier based on the name of the current user.
int filter_string_user_identifier (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  string username = request->session_logic()->currentUser ();
  string hash = md5 (username).substr (0, 5);
  int identifier = config::logic::my_stoi (hash, nullptr, 36);
  return identifier;
}


// C++ equivalent for PHP's bin2hex function.
string bin2hex (string bin)
{
  string res;
  const char hex[] = "0123456789abcdef";
  for (auto sc : bin)
  {
    unsigned char c = static_cast<unsigned char>(sc);
    res += hex[c >> 4];
    res += hex[c & 0xf];
  }
  return res;
}


// C++ equivalent for PHP's hex2bin function.
string hex2bin (string hex)
{
  string out;
  if (hex.length() % 2 == 0) {
    out.reserve (hex.length()/2);
    string extract;
    for (string::const_iterator pos = hex.begin(); pos < hex.end(); pos += 2)
    {
      extract.assign (pos, pos+2);
      out.push_back (static_cast<char> (config::logic::my_stoi (extract, nullptr, 16)));
    }
  }
  return out;
}


// Tidies up html.
string html_tidy (string html)
{
  html = filter_string_str_replace ("<", "\n<", html);
  return html;
}


// Converts elements from the HTML specification to the XML spec.
string html2xml (string html)
{
  // HTML specification: <hr>, XML specification: <hr/>.
  html = filter_string_str_replace ("<hr>", "<hr/>", html);

  // HTML specification: <br>, XML specification: <br/>.
  html = filter_string_str_replace ("<br>", "<br/>", html);

  return html;
}


// Converts XML character entities, like e.g. "&#xB6;" to normal UTF-8 character, like e.g. "¶".
string convert_xml_character_entities_to_characters (string data)
{
  bool keep_going = true;
  int iterations = 0;
  size_t pos1 = static_cast<size_t>(-1);
  do {
    iterations++;
    pos1 = data.find ("&#x", pos1 + 1);
    if (pos1 == string::npos) {
      keep_going = false;
      continue;
    }
    size_t pos2 = data.find (";", pos1);
    if (pos2 == string::npos) {
      keep_going = false;
      continue;
    }
    string entity = data.substr (pos1 + 3, pos2 - pos1 - 3);
    data.erase (pos1, pos2 - pos1 + 1);
    int codepoint;
    stringstream ss;
    ss << hex << entity;
    ss >> codepoint;
    
    // The following is not available in GNU libstdc++.
    // wstring_convert <codecvt_utf8 <char32_t>, char32_t> conv1;
    // string u8str = conv1.to_bytes (codepoint);
    
    int cp = codepoint;
    // Adapted from: http://www.zedwood.com/article/cpp-utf8-char-to-codepoint.
    char c[5]={ 0x00,0x00,0x00,0x00,0x00 };
    if (cp<=0x7F) {
      c[0] = static_cast<char> (cp);
    }
    else if (cp<=0x7FF) {
      c[0] = static_cast<char>((cp>>6)+192);
      c[1] = static_cast<char>((cp&63)+128);
    }
    else if (0xd800<=cp && cp<=0xdfff) {} // Invalid block of utf8.
    else if (cp<=0xFFFF) {
      c[0] = static_cast<char>((cp>>12)+224);
      c[1] = static_cast<char>(((cp>>6)&63)+128);
      c[2] = static_cast<char>((cp&63)+128);
    }
    else if (cp<=0x10FFFF) {
      c[0] = static_cast<char>((cp>>18)+240);
      c[1] = static_cast<char>(((cp>>12)&63)+128);
      c[2] = static_cast<char>(((cp>>6)&63)+128);
      c[3] = static_cast<char>((cp&63)+128);
    }
    string u8str = string (c);
    
    data.insert (pos1, u8str);
  } while (keep_going & (iterations < 100000));
  return data;
}


// Encrypts the $data if the data is unencrypted.
// Decrypts the $data if the data is encrypted.
string encrypt_decrypt (string key, string data)
{
  // Encrypt the key.
  key = md5 (key);
  // Encrypt the data through the encrypted key.
  for (size_t i = 0; i < data.size(); i++) {
    data[i] = data[i] ^ key [i % key.length ()];
  }
  // Result.
  return data;
}


// Gets a new random string for sessions, encryption, you name it.
string get_new_random_string ()
{
  string u = convert_to_string (filter::date::numerical_microseconds ());
  string s = convert_to_string (filter::date::seconds_since_epoch ());
  string r = convert_to_string (config_globals_int_distribution (config_globals_random_engine));
  return md5 (u + s + r);
}


string unicode_non_breaking_space_entity ()
{
  return "&nbsp;";
}


string unicode_black_up_pointing_triangle ()
{
  return "▲";
}


string unicode_black_right_pointing_triangle ()
{
  return "▶";
}


string unicode_black_down_pointing_triangle ()
{
  return "▼";
}


string unicode_black_left_pointing_triangle ()
{
  return "◀";
}


string emoji_black_right_pointing_triangle ()
{
  return "▶️";
}


string emoji_file_folder ()
{
  return "📁";
}


string emoji_open_book ()
{
  return "📖";
}


string emoji_wastebasket ()
{
  return "🗑";
}


string emoji_smiling_face_with_smiling_eyes ()
{
  return "😊";
}


string emoji_heavy_plus_sign ()
{
  return "➕";
}


// Move the $item $up (towards the beginning), or else down (towards the end).
void array_move_up_down (vector <string> & container, size_t item, bool up)
{
  if (container.empty ()) return;
  if (up) {
    if (item > 0) {
      if (item < container.size ()) {
        string s = container [item - 1];
        container [item - 1] = container [item];
        container [item] = s;
      }
    }
  } else {
    if (item < (container.size () - 1)) {
      string s = container [item + 1];
      container [item + 1] = container [item];
      container [item] = s;
    }
  }
}


// Move the item in the $container from position $from to position $to.
void array_move_from_to (vector <string> & container, size_t from, size_t to)
{
  // Check on validity of where moving from and where moving to.
  if (from == to) return;
  if (from >= container.size ()) return;
  if (to >= container.size ()) return;
  
  // Put the data into a map where the keys are multiplied by two.
  map <int, string> mapped_container;
  for (unsigned int i = 0; i < container.size(); i++) {
    mapped_container [static_cast<int>(i * 2)] = container [i];
  }

  // Direction of moving.
  bool move_up = to > from;
  
  // Updated keys.
  from *= 2;
  to *= 2;
  
  // Remove the item, and insert it by a key that puts it at the desired position.
  string moving_item = mapped_container [static_cast<int> (from)];
  mapped_container.erase (static_cast<int> (from));
  if (move_up) to++;
  else to--;
  mapped_container [static_cast<int> (to)] = moving_item;
  
  // Since the map sorts by key,
  // transfer its data back to the original container.
  container.clear ();
  for (auto & element : mapped_container) {
    container.push_back (element.second);
  }
}


const char * english ()
{
  return "English";
}


#ifdef HAVE_WINDOWS
wstring string2wstring(const string& str)
{
  using convert_typeX = codecvt_utf8<wchar_t>;
  wstring_convert<convert_typeX, wchar_t> converterX;
  return converterX.from_bytes(str);
}
#endif


#ifdef HAVE_WINDOWS
string wstring2string(const wstring& wstr)
{
  using convert_typeX = codecvt_utf8<wchar_t>;
  wstring_convert<convert_typeX, wchar_t> converterX;
  return converterX.to_bytes(wstr);
}
#endif


// Converts any line feed character in $str to carriage return + line feed characters,
// basically adding the appropriate carriage return characters.
string lf2crlf (string str)
{
  return filter_string_str_replace ("\n", "\r\n", str);
}


// Converts any carriage return + line feed characters to a line feed character,
// essentially removing any carriage return characters.
string crlf2lf (string str)
{
  return filter_string_str_replace ("\r\n", "\n", str);
}


// Gets the <element> ... </element> part of the input $html.
string filter_text_html_get_element (string html, string element)
{
  size_t pos = html.find ("<" + element);
  if (pos != string::npos) {
    html.erase (0, pos);
    pos = html.find ("</" + element + ">");
    if (pos != string::npos) {
      html.erase (pos + 7);
    }
  }
  return html;
}


/*
string filter_string_tidy_invalid_html_leaking (string html)
{
  // Everything in the <head> can be left out: It is not relevant.
  filter_string_replace_between (html, "<head>", "</head>", "");
  
  // Every <script...</script> can be left out: They are irrelevant.
  int counter = 0;
  while (counter < 100) {
    counter++;
    bool replaced = filter_string_replace_between (html, "<script", "</script>", "");
    if (!replaced) break;
  }
  
#ifdef HAVE_CLOUD

  // This method works via libxml2 and there are many memory leaks each call to this.
  // It cannot be used for production code.
  // The leaks are fixable, see the laboratory/tiny code.
  
  // Create a parser context.
  htmlParserCtxtPtr parser = htmlCreatePushParserCtxt (nullptr, nullptr, nullptr, 0, nullptr, XML_CHAR_ENCODING_UTF8);
  
  // Set relevant options on the parser context.
  htmlCtxtUseOptions(parser, HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);

  // Parse the (X)HTML text.
  // char * data : buffer containing part of the web page
  // int len : number of bytes in data
  // Last argument is 0 if the web page isn't complete, and 1 for the final call.
  htmlParseChunk(parser, html.c_str(), static_cast<int> (html.size()), 1);

  // Extract the fixed html
  if (parser->myDoc) {
    xmlChar *s;
    int size;
    xmlDocDumpMemory(parser->myDoc, &s, &size);
    html = reinterpret_cast<char *> (s);
    xmlFree(s);
  }
  
  // Free memory.
  if (parser) xmlFree (parser);

#endif

  return html;
}
*/


string nonbreaking_inline_tags {"|a|abbr|acronym|b|bdo|big|cite|code|dfn|em|font|i|img|kbd|nobr|s|small|span|strike|strong|sub|sup|tt|"};
string empty_tags {"|area|base|basefont|bgsound|br|command|col|embed|event-source|frame|hr|image|img|input|keygen|link|menuitem|meta|param|source|spacer|track|wbr|"};
string preserve_whitespace_tags {"|pre|textarea|script|style|"};
string special_handling_tags {"|html|body|"};
string no_entity_substitution_tags {"|script|style|"};
string treat_like_inline_tags {"|p|"};


static string substitute_xml_entities_into_text(const string &text)
{
  string result {text};
  // Replacing & must come first.
  result = filter_string_str_replace ("&", "&amp;", result);
  result = filter_string_str_replace ("<", "&lt;", result);
  result = filter_string_str_replace (">", "&gt;", result);
  // Done.
  return result;
}


static string substitute_xml_entities_into_attributes(char quote, const string &text)
{
  string result {substitute_xml_entities_into_text (text)};
  if (quote == '"') {
    result = filter_string_str_replace("\"","&quot;", result);
  }
  else if (quote == '\'') {
    result = filter_string_str_replace("'","&apos;", result);
  }
  return result;
}


#ifdef HAVE_CLOUD
static string handle_unknown_tag(GumboStringPiece *text)
{
  string tagname {};
  if (text->data == NULL) {
    return tagname;
  }
  // work with copy GumboStringPiece to prevent asserts
  // if try to read same unknown tag name more than once
  GumboStringPiece gsp = *text;
  gumbo_tag_from_original_text(&gsp);
  tagname = string(gsp.data, gsp.length);
  return tagname;
}
#endif


#ifdef HAVE_CLOUD
static string get_tag_name(GumboNode *node)
{
  string tagname;
  // work around lack of proper name for document node
  if (node->type == GUMBO_NODE_DOCUMENT) {
    tagname = "document";
  } else {
    tagname = gumbo_normalized_tagname(node->v.element.tag);
  }
  if (tagname.empty()) {
    tagname = handle_unknown_tag(&node->v.element.original_tag);
  }
  return tagname;
}
#endif


#ifdef HAVE_CLOUD
static string build_doctype(GumboNode *node)
{
  string results {};
  if (node->v.document.has_doctype) {
    results.append("<!DOCTYPE ");
    results.append(node->v.document.name);
    string pi(node->v.document.public_identifier);
    if ((node->v.document.public_identifier != NULL) && !pi.empty() ) {
      results.append(" PUBLIC \"");
      results.append(node->v.document.public_identifier);
      results.append("\" \"");
      results.append(node->v.document.system_identifier);
      results.append("\"");
    }
    results.append(">\n");
  }
  return results;
}
#endif


#ifdef HAVE_CLOUD
static string build_attributes(GumboAttribute * at, bool no_entities)
{
  string atts {};
  atts.append (" ");
  atts.append (at->name);
  
  // how do we want to handle attributes with empty values
  // <input type="checkbox" checked />  or <input type="checkbox" checked="" />
  
  if ( (!string(at->value).empty())   ||
      (at->original_value.data[0] == '"') ||
      (at->original_value.data[0] == '\'') ) {
    
    // determine original quote character used if it exists
    char quote = at->original_value.data[0];
    string qs = "";
    if (quote == '\'') qs = string("'");
    if (quote == '"') qs = string("\"");
    
    atts.append("=");
    
    atts.append(qs);
    
    if (no_entities) {
      atts.append(at->value);
    } else {
      atts.append(substitute_xml_entities_into_attributes(quote, string(at->value)));
    }
    
    atts.append(qs);
  }
  return atts;
}
#endif


// Forward declaration
#ifdef HAVE_CLOUD
static string pretty_print (GumboNode*, int lvl, const string & indent_chars);
#endif


// Pretty-print children of a node. May be invoked recursively.
#ifdef HAVE_CLOUD
static string pretty_print_contents (GumboNode* node, int lvl, const string & indent_chars)
{
  string contents {};
  string tagname {get_tag_name(node)};
  string key {"|" + tagname + "|"};
  bool no_entity_substitution {no_entity_substitution_tags.find(key) != string::npos};
  bool keep_whitespace {preserve_whitespace_tags.find(key) != string::npos};
  bool is_inline {nonbreaking_inline_tags.find(key) != string::npos};
  bool pp_okay {!is_inline && !keep_whitespace};
  
  GumboVector* children {&node->v.element.children};
  
  for (unsigned int i {0}; i < children->length; ++i) {
    GumboNode* child = static_cast<GumboNode*> (children->data[i]);
    
    if (child->type == GUMBO_NODE_TEXT) {
      
      string val {};
      
      if (no_entity_substitution) {
        val = string(child->v.text.text);
      } else {
        val = substitute_xml_entities_into_text(string(child->v.text.text));
      }
      
      if (pp_okay) {
        val = filter_string_rtrim(val);
      }
      
      if (pp_okay && (contents.length() == 0)) {
        // Add the required indentation.
        char c {indent_chars.at(0)};
        size_t n {indent_chars.length()};
        contents.append (string (static_cast<size_t>(lvl-1)*n,c));
      }
      
      contents.append (val);
      
      
    } else if ((child->type == GUMBO_NODE_ELEMENT) || (child->type == GUMBO_NODE_TEMPLATE)) {
      
      string val = pretty_print(child, lvl, indent_chars);
      
      // Remove any indentation if this child is inline and not a first child.
      string childname = get_tag_name(child);
      string childkey = "|" + childname + "|";
      if ((nonbreaking_inline_tags.find(childkey) != string::npos) && (contents.length() > 0)) {
        val = filter_string_ltrim(val);
      }
      
      contents.append(val);
      
    } else if (child->type == GUMBO_NODE_WHITESPACE) {
      
      if (keep_whitespace || is_inline) {
        contents.append(string(child->v.text.text));
      }
      
    } else if (child->type != GUMBO_NODE_COMMENT) {
      
      // Does this actually exist: (child->type == GUMBO_NODE_CDATA)
      fprintf(stderr, "unknown element of type: %d\n", child->type);
      
    }
    
  }
  
  return contents;
}
#endif


// Pretty-print a GumboNode back to html/xhtml. May be invoked recursively
#ifdef HAVE_CLOUD
static string pretty_print(GumboNode* node, int lvl, const string & indent_chars)
{
  // Special case: The document node.
  if (node->type == GUMBO_NODE_DOCUMENT) {
    string results {build_doctype(node)};
    results.append(pretty_print_contents (node, lvl + 1, indent_chars));
    return results;
  }
  
  string close {};
  string closeTag {};
  string attributes {};
  string tagname {get_tag_name(node)};
  string key {"|" + tagname + "|"};
  //bool need_special_handling {special_handling.find(key) != string::npos};
  bool is_empty_tag {empty_tags.find(key) != string::npos};
  bool no_entity_substitution {no_entity_substitution_tags.find(key) != string::npos};
  bool keep_whitespace {preserve_whitespace_tags.find(key) != string::npos};
  bool is_inline {nonbreaking_inline_tags.find(key) != string::npos};
  bool inline_like {treat_like_inline_tags.find(key) != string::npos};
  bool pp_okay {!is_inline && !keep_whitespace};
  char c {indent_chars.at(0)};
  size_t n {indent_chars.length()};
  
  // Build the attr string.
  const GumboVector * attribs {&node->v.element.attributes};
  for (unsigned int i = 0; i < attribs->length; ++i) {
    GumboAttribute * gumbo_attribute {static_cast<GumboAttribute*>(attribs->data[i])};
    attributes.append (build_attributes (gumbo_attribute, no_entity_substitution));
  }
  
  // Determine the closing tag type.
  if (is_empty_tag) {
    close = "/";
  } else {
    closeTag = "</" + tagname + ">";
  }
  
  string indent_space {string (static_cast<size_t>(lvl-1)*n,c)};
  
  // Pretty print the contents.
  string contents {pretty_print_contents(node, lvl+1, indent_chars)};
  
//  if (need_special_handling) {
//    contents = filter_string_rtrim(contents);
//  }
  
  char last_char = ' ';
  if (!contents.empty()) {
    last_char = contents.at (contents.length() - 1);
  }
  
  // Build the results.
  string results;
  if (pp_okay) {
    results.append(indent_space);
  }
  results.append("<"+tagname+attributes+close+">");
  if (pp_okay && !inline_like) {
    results.append("\n");
  }
//  if (inline_like) {
//    contents = filter_string_ltrim(contents);
//  }
  results.append(contents);
  if (pp_okay && !contents.empty() && (last_char != '\n') && (!inline_like)) {
    results.append("\n");
  }
  if (pp_okay && !inline_like && !closeTag.empty()) {
    results.append(indent_space);
  }
  results.append(closeTag);
  if (pp_okay && !closeTag.empty()) {
    results.append("\n");
  }
  
  return results;
}
#endif


string filter_string_fix_invalid_html_gumbo (string html)
{
  // Everything in the <head> can be left out: It is not relevant.
  filter_string_replace_between (html, "<head>", "</head>", string());
  
  // Every <script...</script> can be left out: They are irrelevant.
  int counter {0};
  while (counter < 100) {
    counter++;
    bool replaced = filter_string_replace_between (html, "<script", "</script>", string());
    if (!replaced) break;
  }
  
#ifdef HAVE_CLOUD

  // https://github.com/google/gumbo-parser
  GumboOptions options {kGumboDefaultOptions};
  GumboOutput* output = gumbo_parse_with_options(&options, html.data(), html.length());
  string indent_chars {" "};
  html = pretty_print (output->document, 0, indent_chars);
  gumbo_destroy_output (&options, output);
  
#endif
  
  return html;
}


string filter_string_fix_invalid_html_tidy (string html)
{
#ifdef HAVE_CLOUD

  // The buffers.
  TidyBuffer output {};
  memset (&output, 0, sizeof(TidyBuffer));
  TidyBuffer errbuf {};
  memset (&errbuf, 0, sizeof(TidyBuffer));
  
  // Initialize the document.
  TidyDoc tdoc = tidyCreate();
  
  // Set a few options.
  tidyOptSetBool (tdoc, TidyXmlOut, yes);
  tidyOptSetBool (tdoc, TidyHideComments, yes);
  tidyOptSetBool (tdoc, TidyJoinClasses, yes);
  tidyOptSetBool (tdoc, TidyBodyOnly, yes);

  // Capture diagnostics.
  int rc = tidySetErrorBuffer (tdoc, &errbuf);

  // Parse the input.
  if (rc >= 0) rc = tidyParseString (tdoc, html.c_str());

  // Tidy it up.
  if (rc >= 0) rc = tidyCleanAndRepair (tdoc);

  // Run the diagnostics.
  if (rc >= 0) rc = tidyRunDiagnostics (tdoc);
  // If error, force output.
  if (rc > 1) rc = (tidyOptSetBool (tdoc, TidyForceOutput, yes) ? rc : -1);

  // Pretty print.
  if (rc >= 0) rc = tidySaveBuffer (tdoc, &output);
  
  if (rc >= 0) {
    if (rc > 0) {
//      cerr << "Html tidy diagnostics:" << endl;
//      cerr << errbuf.bp << endl;
    }
//    cout << "Html tidy result:" << endl;
//    cout << output.bp << endl;
    html = string (reinterpret_cast<char const*>(output.bp));
  }
  else {
    Database_Logs::log("A severe error occurred while tidying html - code " + to_string(rc) + " - html: " + html);
  }
  
  // Release memory.
  tidyBufFree (&output);
  tidyBufFree (&errbuf);
  tidyRelease (tdoc);

#endif

  // Done.
  return html;
}


string filter_string_collapse_whitespace (string s)
{
  int count;
  int iterator {0};
  do {
    count = 0;
    s = filter_string_str_replace ("  ", " ", s, &count);
    iterator++;
  } while ((count > 0) && iterator < 5);
  return s;
}


std::string convert_windows1252_to_utf8 (const std::string& input)
{
  // Convert the encoding.
  string utf8 {};
  utf8::utf16to8(input.begin(), input.end(), back_inserter(utf8));

  // Handle weird conversions.
  utf8 = filter_string_str_replace ("￯﾿ﾽ", "'", utf8);

  // Pass it to the caller.
  return utf8;

  // It could be done through the iconv library.
  // in the way that the iconv binary does it.
  // 1, Remove the meta information from the html head.
  // 2. iconv -f CP1252 -t UTF-8 ./john-ma-lbw2.htm > john-ma-lbw3.htm
  // But libiconv-dev is not available as a Debian package.
  // So eventually libiconv was not used.

  // The conversion descriptor for converting WINDOWS-1252 -> UTF-8.
  //iconv_t conversion_descriptor = iconv_open ("UTF-8", "CP1252");
  //if (conversion_descriptor == (iconv_t)(-1)) {
  //  throw std::runtime_error("Cannot open converter from Windows-1252 to UTF-8");
  //}
    
  // The pointer to the input buffer.
  //char* input_pointer = const_cast<char*>(input.c_str());
    
  // The output buffer.
  //constexpr int outbuf_size {10000}; // make it dynamic.
  //unsigned char outbuf[outbuf_size];
  //memset(outbuf, 0, outbuf_size);
  //char *outptr = (char*) outbuf;
  
  // The number of bytes left in the input and output buffers.
  //size_t input_bytes_left {input.length()};
  //size_t output_bytes_left {outbuf_size};

  // Repeat converting and handling unconvertible characters.
  //while (input_bytes_left > 0) {
    // Do the conversion.
    //size_t result = iconv(conversion_descriptor, &input_pointer, &input_bytes_left, &outptr, &output_bytes_left);
    //if (result == (size_t)(-1)) {
      // Handle situation that an invalid multibyte sequence is encountered in the input.
      // In this case the input pointer is left pointing to
      // the beginning of the invalid multibyte sequence.
      //if (errno == EILSEQ) {
      //  int one = 1;
      //  iconvctl (conversion_descriptor, ICONV_SET_DISCARD_ILSEQ, &one);
      //} else if (errno == EINVAL) {
      //  int one = 1;
      //  iconvctl (conversion_descriptor, ICONV_SET_DISCARD_ILSEQ, &one);
      //} else if (errno == E2BIG) {
      //  input_bytes_left = 0;
      //} else {
      //  input_bytes_left = 0;
      //}
    //}
  //}
  
  // Close the conversion descriptor.
  //iconv_close(conversion_descriptor);
  
  // Assemble the resulting UTF-8 text.
  //std::string utf8 {};
  //utf8.assign ((char*)outbuf, outbuf_size - output_bytes_left);
  
  // Handle weird conversions.
  //utf8 = filter_string_str_replace ("ï¿½", "'", utf8);
  
  // Pass it to the caller.
  //return utf8;
}
