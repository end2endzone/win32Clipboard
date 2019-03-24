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

  /// <summary>Converts the encoding of the given unicode string to utf8.</summary>
  /// <param name="wstr">The unicode encoded string.</param>
  /// <returns>An utf8 encoded string.</returns>
  /// <remarks>The function is inspired by https://gist.github.com/pezy/8571764 </remarks>
  std::string unicode_to_utf8(const std::wstring & wstr);

  /// <summary>Converts the encoding of the given utf8 string to unicode.</summary>
  /// <param name="wstr">The utf8 encoded string.</param>
  /// <returns>An unicode encoded string.</returns>
  /// <remarks>The function is inspired by https://gist.github.com/pezy/8571764 </remarks>
  std::wstring utf8_to_unicode(const std::string & str);

  /// <summary>Converts the encoding of the given unicode string to ansi.</summary>
  /// <param name="wstr">The unicode encoded string.</param>
  /// <returns>An ansi encoded string.</returns>
  /// <remarks>The function is inspired by https://gist.github.com/pezy/8571764 </remarks>
  std::string unicode_to_ansi(const std::wstring & wstr);

  /// <summary>Converts the encoding of the given utf8 string to ansi.</summary>
  /// <param name="wstr">The ansi encoded string.</param>
  /// <returns>An unicode encoded string.</returns>
  /// <remarks>The function is inspired by https://gist.github.com/pezy/8571764 </remarks>
  std::wstring ansi_to_unicode(const std::string & str);
  

  class Clipboard
  {
  private:
    Clipboard();
    virtual ~Clipboard();

  public:
    static Clipboard & getInstance();

    //enums
    enum Format { FormatText, FormatImage, FormatBinary }; 
    enum DragDropType {DragDropCopy, DragDropCut};

    //typedefs
    typedef std::vector<std::string> StringVector;
    typedef std::string MemoryBuffer;

    //constants
    static const size_t NUM_FORMATS = 3;

    virtual void empty();
    virtual bool isEmpty();
    virtual bool contains(Format iClipboardFormat);
    virtual bool setText(const std::string & iText);
    virtual bool getAsText(std::string & oText); 
    virtual bool setBinary(const MemoryBuffer & iMemoryBuffer);
    virtual bool getAsBinary(MemoryBuffer & oMemoryBuffer);
    virtual bool setDragDropFiles(const DragDropType & iDragDropType, const StringVector & iFiles);
    virtual bool getAsDragDropFiles(DragDropType & oDragDropType, StringVector & oFiles);

  };

} //namespace win32clipboard

#endif //WIN32CLIPBOARD_H
