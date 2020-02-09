/**********************************************************************************
 * MIT License
 * 
 * Copyright (c) 2018 Antoine Beauchamp
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *********************************************************************************/

#ifndef WIN32CLIPBOARD_H
#define WIN32CLIPBOARD_H

#include <vector>
#include <string>

namespace win32clipboard
{
  //global functions

  /// <summary>
  /// Returns true if the given string is encoded in ASCII.
  /// </summary>
  /// <param name="str">The buffer of the given string.</param>
  /// <returns>Returns true if the given string is encoded in ASCII. Returns false otherwise</returns>
  bool is_ascii(const char * str);

  /// <summary>
  /// Returns true if the given string is compatible with Windows CP 1252 encoding.
  /// </summary>
  /// <param name="str">The buffer of the given string.</param>
  /// <returns>Returns true if the given string is compatible with Windows CP 1252 encoding. Returns false otherwise</returns>
  bool is_cp1252_valid(const char * str);

  /// <summary>
  /// Returns true if the given string is compatible with ISO-8859-1 encoding.
  /// </summary>
  /// <param name="str">The buffer of the given string.</param>
  /// <returns>Returns true if the given string is compatible with ISO-8859-1 encoding. Returns false otherwise</returns>
  /// <remarks>
  /// A buffer that is compatible with ISO-8859-1 encoding will always be compatible with Windows CP 1252 encoding.
  /// In other words, all characters of ISO-8859-1 encoding are also available in Windows CP 1252 encoding.
  /// See https://en.wikipedia.org/wiki/Windows-1252#Code_page_layout and
  /// https://en.wikipedia.org/wiki/ISO/IEC_8859-1#Code_page_layout for details.
  /// </remarks>
  bool is_iso8859_1_valid(const char * str);

  /// <summary>
  /// Returns true if the given string is compatible with UTF-8 encoding.
  /// </summary>
  /// <param name="str">The buffer of the given string.</param>
  /// <returns>Returns true if the given string is compatible with UTF-8 encoding. Returns false otherwise</returns>
  /// <remarks>A buffer that is pure ASCII will always be compatible with UTF-8 encoding.</remarks>
  bool is_utf8_valid(const char * str);

  /// <summary>
  /// Convert a wide-character-unicode string to an utf8-encoded string.
  /// </summary>
  /// <param name="wstr">The wide-character-unicode string to convert.</param>
  /// <returns>Returns an utf8-encoded string. Returns an empty string on failure.</returns>
  /// <remarks>If a non-empty input string is given as input, an empty output string must be considered a decoding or encoding error.< / remarks>
  std::string unicode_to_utf8(const std::wstring & wstr);
 
  /// <summary>
  /// Convert an utf8-encoded string to a wide-character-unicode string.
  /// </summary>
  /// <param name="str">The utf8-encoded string to convert.</param>
  /// <returns>Returns a wide-character-unicode string. Returns an empty string on failure.</returns>
  /// <remarks>If a non-empty input string is given as input, an empty output string must be considered a decoding or encoding error.< / remarks>
  std::wstring utf8_to_unicode(const std::string & str);
 
  /// <summary>
  /// Convert a wide-character-unicode string to an ansi-encoded string.
  /// </summary>
  /// <param name="str">The wide-character-unicode string to convert.</param>
  /// <returns>Returns an ansi-encoded string. Returns an empty string on failure.</returns>
  /// <remarks>If a non-empty input string is given as input, an empty output string must be considered a decoding or encoding error.< / remarks>
  std::string unicode_to_ansi(const std::wstring & wstr);
 
  /// <summary>
  /// Convert an ansi-encoded string to a wide-character-unicode string.
  /// </summary>
  /// <param name="str">The ansi-encoded string to convert.</param>
  /// <returns>Returns a wide-character-unicode string. Returns an empty string on failure.</returns>
  /// <remarks>If a non-empty input string is given as input, an empty output string must be considered a decoding or encoding error.< / remarks>
  std::wstring ansi_to_unicode(const std::string & str);
 
  /// <summary>
  /// Convert an utf8-encoded string to an ansi-encoded string.
  /// </summary>
  /// <param name="str">The utf8-encoded string to convert.</param>
  /// <returns>Returns an ansi-encoded string. Returns an empty string on failure.</returns>
  /// <remarks>If a non-empty input string is given as input, an empty output string must be considered a decoding or encoding error.< / remarks>
  std::string utf8_to_ansi(const std::string & str);
 
  /// <summary>
  /// Convert an ansi-encoded string to an utf8-encoded string.
  /// </summary>
  /// <param name="str">The ansi-encoded string to convert.</param>
  /// <returns>Returns an utf8-encoded string. Returns an empty string on failure.</returns>
  /// <remarks>If a non-empty input string is given as input, an empty output string must be considered a decoding or encoding error.< / remarks>
  std::string ansi_to_utf8(const std::string & str);
 
  class Clipboard
  {
  private:
    Clipboard();
    virtual ~Clipboard();

  public:
    static Clipboard & getInstance();

    //enums
    enum Format { FormatText, FormatUnicode, FormatImage, FormatBinary }; 
    enum DragDropType {DragDropCopy, DragDropCut};

    //typedefs
    typedef std::vector<std::string> StringVector;
    typedef std::string MemoryBuffer;

    //constants
    static const size_t NUM_FORMATS = 3;

    virtual bool empty();
    virtual bool isEmpty();
    virtual bool contains(Format iClipboardFormat);
    virtual bool setText(const std::string & iText);
    virtual bool getAsText(std::string & oText); 
    virtual bool setUnicode(const std::wstring & iText);
    virtual bool getAsUnicode(std::wstring & oText); 
    virtual bool setBinary(const MemoryBuffer & iMemoryBuffer);
    virtual bool getAsBinary(MemoryBuffer & oMemoryBuffer);
    virtual bool setDragDropFiles(const DragDropType & iDragDropType, const StringVector & iFiles);
    virtual bool getAsDragDropFiles(DragDropType & oDragDropType, StringVector & oFiles);

  };

} //namespace win32clipboard

#endif //WIN32CLIPBOARD_H
