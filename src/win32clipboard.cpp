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

#include <windows.h>
#undef min
#undef max

#include <shlobj.h>
#include <shellApi.h>

#include "win32clipboard/win32clipboard.h"

#include "rapidassist/strings.h"
#include "rapidassist/environment.h"
#include "rapidassist/timing.h"

namespace win32clipboard
{
  //Format descriptors for different types of object sent to the clipboard
  static const UINT gFormatDescriptorBinary     = RegisterClipboardFormat("Binary");
  static const UINT gFormatDescriptorDropEffect = RegisterClipboardFormat("Preferred DropEffect");

  static const std::string CRLF = ra::environment::GetLineSeparator();
  static const std::string  EMPTY_STRING;
  static const std::wstring EMPTY_WIDE_STRING;

  #define DEFAULT_READ_CLIPBOARD_HANDLE   NULL
  #define DEFAULT_WRITE_CLIPBOARD_HANDLE  GetDesktopWindow()

  bool is_ascii(const char * str)
  {
    int offset = 0;
    while (str[offset] != '\0')
    {
      if (str[offset] < 0) //if bit7 is set
        return false;

      //next byte
      offset++;
    }
    return true;
  }

  bool is_cp1252_valid(const char * str)
  {
    int offset = 0;
    while (str[offset] != '\0')
    {
      const char & c = str[offset];
      if (
        c == 0x81 ||
        c == 0x8D ||
        c == 0x8F ||
        c == 0x90 ||
        c == 0x9D )
        return false;

      //next byte
      offset++;
    }
    return true;
  }

  bool is_iso8859_1_valid(const char * str)
  {
    int offset = 0;
    while (str[offset] != '\0')
    {
      const char & c = str[offset];
      if (0x00 <= c && c <= 0x1F)
        return false;
      if (0x7F <= c && c <= 0x9F)
        return false;

      //next byte
      offset++;
    }
    return true;
  }

  bool is_utf8_valid(const char * str)
  {
    int offset = 0;
    while (str[offset] != '\0')
    {
      const char & c1 = str[offset + 0];
      char c2 = str[offset + 1];
      char c3 = str[offset + 2];
      char c4 = str[offset + 3];
    
      //prevent going outside of the string
      if (c1 == '\0')
        c2 = c3 = c4 = '\0';
      else if (c2 == '\0')
        c3 = c4 = '\0';
      else if (c3 == '\0')
        c4 = '\0';

      //size in bytes of the code point
      int n = 1;

      //See http://www.unicode.org/versions/Unicode6.0.0/ch03.pdf, Table 3-7. Well-Formed UTF-8 Byte Sequences
      // ## | Code Points         | First Byte | Second Byte | Third Byte | Fourth Byte
      // #1 | U+0000   - U+007F   | 00 - 7F    |             |            | 
      // #2 | U+0080   - U+07FF   | C2 - DF    | 80 - BF     |            | 
      // #3 | U+0800   - U+0FFF   | E0         | A0 - BF     | 80 - BF    | 
      // #4 | U+1000   - U+CFFF   | E1 - EC    | 80 - BF     | 80 - BF    | 
      // #5 | U+D000   - U+D7FF   | ED         | 80 - 9F     | 80 - BF    | 
      // #6 | U+E000   - U+FFFF   | EE - EF    | 80 - BF     | 80 - BF    | 
      // #7 | U+10000  - U+3FFFF  | F0         | 90 - BF     | 80 - BF    | 80 - BF
      // #8 | U+40000  - U+FFFFF  | F1 - F3    | 80 - BF     | 80 - BF    | 80 - BF
      // #9 | U+100000 - U+10FFFF | F4         | 80 - 8F     | 80 - BF    | 80 - BF

      if (c1 <= 0x7F) // #1 | U+0000   - U+007F, (ASCII)
        n = 1;
      else if ( 0xC2 <= c1 && c1 <= 0xDF &&
                0x80 <= c2 && c2 <= 0xBF)  // #2 | U+0080   - U+07FF
        n = 2;
      else if ( 0xE0 == c1 &&
                0xA0 <= c2 && c2 <= 0xBF &&
                0x80 <= c3 && c3 <= 0xBF)  // #3 | U+0800   - U+0FFF
        n = 3;
      else if ( 0xE1 <= c1 && c1 <= 0xEC &&
                0x80 <= c2 && c2 <= 0xBF &&
                0x80 <= c3 && c3 <= 0xBF)  // #4 | U+1000   - U+CFFF
        n = 3;
      else if ( 0xED == c1 &&
                0x80 <= c2 && c2 <= 0x9F &&
                0x80 <= c3 && c3 <= 0xBF)  // #5 | U+D000   - U+D7FF
        n = 3;
      else if ( 0xEE <= c1 && c1 <= 0xEF &&
                0x80 <= c2 && c2 <= 0xBF &&
                0x80 <= c3 && c3 <= 0xBF)  // #6 | U+E000   - U+FFFF
        n = 3;
      else if ( 0xF0 == c1 &&
                0x90 <= c2 && c2 <= 0xBF &&
                0x80 <= c3 && c3 <= 0xBF &&
                0x80 <= c4 && c4 <= 0xBF)  // #7 | U+10000  - U+3FFFF
        n = 4;
      else if ( 0xF1 <= c1 && c1 <= 0xF3 &&
                0x80 <= c2 && c2 <= 0xBF &&
                0x80 <= c3 && c3 <= 0xBF &&
                0x80 <= c4 && c4 <= 0xBF)  // #8 | U+40000  - U+FFFFF
        n = 4;
      else if ( 0xF4 == c1 &&
                0x80 <= c2 && c2 <= 0xBF &&
                0x80 <= c3 && c3 <= 0xBF &&
                0x80 <= c4 && c4 <= 0xBF)  // #7 | U+10000  - U+3FFFF
        n = 4;
      else
        return false; // invalid UTF-8 sequence

      //next code point
      offset += n;
    }
    return true;
  }

  // Convert a wide Unicode string to an UTF8 string
  std::string unicode_to_utf8(const std::wstring & wstr)
  {
    if (wstr.empty()) return std::string();
    int num_characters = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    if (num_characters == 0)
      return EMPTY_STRING;
    std::string strTo(num_characters, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], num_characters, NULL, NULL);
    return strTo;
  }

  // Convert an UTF8 string to a wide Unicode String
  std::wstring utf8_to_unicode(const std::string & str)
  {
    if (str.empty()) return std::wstring();
    int num_characters = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    if (num_characters == 0)
      return EMPTY_WIDE_STRING;
    std::wstring wstrTo(num_characters, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], num_characters);
    return wstrTo;
  }

  // Convert an wide Unicode string to ANSI string
  std::string unicode_to_ansi(const std::wstring & wstr)
  {
    if (wstr.empty()) return std::string();
    int num_characters = WideCharToMultiByte(CP_ACP, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    if (num_characters == 0)
      return EMPTY_STRING;
    std::string strTo(num_characters, 0);
    WideCharToMultiByte(CP_ACP, 0, &wstr[0], (int)wstr.size(), &strTo[0], num_characters, NULL, NULL);
    return strTo;
  }

  // Convert an ANSI string to a wide Unicode String
  std::wstring ansi_to_unicode(const std::string & str)
  {
    if (str.empty()) return std::wstring();
    int num_characters = MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), NULL, 0);
    if (num_characters == 0)
      return EMPTY_WIDE_STRING;
    std::wstring wstrTo(num_characters, 0);
    MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), &wstrTo[0], num_characters);
    return wstrTo;
  }

  std::string utf8_to_ansi(const std::string & str)
  {
    std::wstring str_unicode = utf8_to_unicode(str);
    std::string str_ansi = unicode_to_ansi(str_unicode);
    return str_ansi;
  }
 
  std::string ansi_to_utf8(const std::string & str)
  {
    std::wstring str_unicode = ansi_to_unicode(str);
    std::string str_utf8 = unicode_to_utf8(str_unicode);
    return str_utf8;
  }
 
  std::string getLastErrorDescription()
  {
    DWORD dwLastError = ::GetLastError();
    char lpErrorBuffer[10240] = {0};
    DWORD dwErrorBufferSize = sizeof(lpErrorBuffer);
    ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    dwLastError,
                    MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
                    lpErrorBuffer,
                    dwErrorBufferSize-1,
                    NULL);
    char lpDescBuffer[10240] = {0};
    sprintf(lpDescBuffer, "Error %d, %s", dwLastError, lpErrorBuffer);
    return std::string(lpDescBuffer);
  }

  class ClipboardObject
  {
  public:
    ClipboardObject(HWND hWnd)
    {
      mHwnd = hWnd;
      mOpenStatus = FALSE;
      mCloseStatus = FALSE;

      //Calling OpenClipboard() following a CloseClipboard() may sometimes fails with "Error 0x00000005, Access is denied."
      //Retry a maximum of 5 times to open the clipboard
      for(size_t i=0; i<5 && mOpenStatus == FALSE; i++)
      {
        mOpenStatus = OpenClipboard(mHwnd);
        if (mOpenStatus == FALSE)
        {
          std::string error = getLastErrorDescription();

          //Failed opening the clipboard object. Will try again little bit later
          ra::timing::Millisleep(50);
        }
      }
    }

    bool isOpened()
    {
      return (mOpenStatus != 0);
    }

    ~ClipboardObject()
    {
      if (isOpened())
      {
        mCloseStatus = CloseClipboard();
      }
    }

  private:
    HWND mHwnd;
    BOOL mOpenStatus;
    BOOL mCloseStatus;
  };

  Clipboard::Clipboard()
  {
  }

  Clipboard::~Clipboard()
  {
  }

  Clipboard & Clipboard::getInstance()
  {
    static Clipboard _instance;
    return _instance;
  }

  bool Clipboard::empty()
  {
    ClipboardObject obj( DEFAULT_WRITE_CLIPBOARD_HANDLE );
    if (!obj.isOpened())
      return false;

    BOOL empty = EmptyClipboard();
    
    return (empty == TRUE);
  }

  bool Clipboard::isEmpty()
  {
    //Check if the clipboard contains any of the known formats
    for(size_t i=0; i<Clipboard::NUM_FORMATS; i++)
    {
      Clipboard::Format test_format = (Clipboard::Format)i;
      if (contains(test_format))
      {
        return false;
      }
    }
    return true;
  }

  bool Clipboard::contains(Clipboard::Format iClipboardFormat)
  {
    ClipboardObject obj( DEFAULT_READ_CLIPBOARD_HANDLE );
    if (!obj.isOpened())
      return false;

    bool containsFormat = false;
    switch(iClipboardFormat)
    {
    case Clipboard::FormatText:
      {
        HANDLE hData = GetClipboardData(CF_TEXT);
        containsFormat = (hData != NULL);
      }
      break;
    case Clipboard::FormatUnicode:
      {
        HANDLE hData = GetClipboardData(CF_UNICODETEXT);
        containsFormat = (hData != NULL);
      }
      break;
    case Clipboard::FormatImage:
      {
        HANDLE hData = GetClipboardData(CF_BITMAP);
        containsFormat = (hData != NULL);
      }
      break;
    case Clipboard::FormatBinary:
      {
        HANDLE hData = GetClipboardData(gFormatDescriptorBinary);
        containsFormat = (hData != NULL);
      }
      break;
    };

    return containsFormat;
  }

  template <typename T> inline bool setTextT(UINT uFormat, const T* str, size_t length)
  {
    ClipboardObject obj( DEFAULT_WRITE_CLIPBOARD_HANDLE );
    if (!obj.isOpened())
      return false;

    //flush existing content
    if (!EmptyClipboard())
      return false;

    size_t memory_size = (length+1)*sizeof(T); // +1 character to include the NULL terminating character

    //copy data to global allocated memory
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, memory_size);
    if (hMem == NULL)
      return false;
    void * buffer = GlobalLock(hMem);
    memcpy(buffer, str, memory_size);
    GlobalUnlock(hMem);

    //put it on the clipboard
    HANDLE hData = SetClipboardData(uFormat, hMem);
    if (hData != hMem)
      return false;

    return true;
  }
 
  bool Clipboard::setText(const std::string & iText)
  {
    return setTextT<char>(CF_TEXT, iText.data(), iText.length());
  }

  bool Clipboard::getAsText(std::string & oText)
  {
    ClipboardObject obj( DEFAULT_READ_CLIPBOARD_HANDLE );
    if (!obj.isOpened())
      return false;

    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == NULL)
      return false;

    size_t data_size = (size_t)GlobalSize(hData);
    size_t count = data_size / sizeof(char);
    const char* text_buffer = (const char*)GlobalLock(hData);
    oText.assign(text_buffer, count-1); //copy the data to output variable, minus the last \0 character
    GlobalUnlock( hData );

    return true;
  }

  bool Clipboard::setUnicode(const std::wstring & iText)
  {
    return setTextT<wchar_t>(CF_UNICODETEXT, iText.data(), iText.length());
  }

  bool Clipboard::getAsUnicode(std::wstring & oText)
  {
    ClipboardObject obj(DEFAULT_READ_CLIPBOARD_HANDLE);
    if (!obj.isOpened())
      return false;

    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (hData == NULL)
      return false;

    size_t data_size = (size_t)GlobalSize(hData);
    size_t count = data_size / sizeof(wchar_t);
    const wchar_t* text_buffer = (const wchar_t*)GlobalLock(hData);
    oText.assign(text_buffer, count- 1); //copy the data to output variable, minus the last \0 character
    GlobalUnlock(hData);

    return true;
  }

  bool Clipboard::setBinary(const MemoryBuffer & iMemoryBuffer)
  {
    ClipboardObject obj( DEFAULT_WRITE_CLIPBOARD_HANDLE );
    if (!obj.isOpened())
      return false;

    //flush existing content
    if (!EmptyClipboard())
      return false;

    //copy data to global allocated memory
    HGLOBAL hMem = GlobalAlloc(GMEM_DDESHARE, iMemoryBuffer.size());
    if (hMem == NULL)
      return false;
    void * buffer = GlobalLock(hMem);
    memcpy(buffer, iMemoryBuffer.c_str(), iMemoryBuffer.size());
    GlobalUnlock(hMem);

    //put it on the clipboard
    HANDLE hData = SetClipboardData(gFormatDescriptorBinary, hMem);
    if (hData != hMem)
      return false;

    return true;
  }

  bool Clipboard::getAsBinary(MemoryBuffer & oMemoryBuffer)
  {
    ClipboardObject obj( DEFAULT_READ_CLIPBOARD_HANDLE );
    if (!obj.isOpened())
      return false;

    //get the buffer
    HANDLE hData = GetClipboardData(gFormatDescriptorBinary);
    if (hData == NULL)
      return false;

    size_t data_size = (size_t)GlobalSize(hData);
    void * buffer = GlobalLock( hData );
    oMemoryBuffer.assign((const char*)buffer, data_size); //copy the data to output variable
    GlobalUnlock( hData );
    
    return true;
  }

  bool Clipboard::setDragDropFiles(const Clipboard::DragDropType & iDragDropType, const Clipboard::StringVector & iFiles)
  {
    //http://support.microsoft.com/kb/231721/en-us
    //http://aclacl.brinkster.net/MFC/ch19b.htm
    //http://read.pudn.com/downloads22/sourcecode/windows/multimedia/73340/ShitClass3/%E7%B1%BB%E5%8C%85/SManageFile.cpp__.htm

    //Validate drag drop type
    if (iDragDropType != Clipboard::DragDropCopy && iDragDropType != Clipboard::DragDropCut)
      return false;

    ClipboardObject obj( DEFAULT_WRITE_CLIPBOARD_HANDLE );
    if (!obj.isOpened())
      return false;

    //flush existing content
    if (!EmptyClipboard())
      return false;

    //Register iFiles
    {
      DROPFILES df = {0};
      df.pFiles = sizeof(DROPFILES);
      df.pt.x = 0;
      df.pt.y = 0;
      df.fNC = FALSE;
      df.fWide = TRUE; //we will use WIDE CHAR for storing the file paths

      //Build the buffer content
      MemoryBuffer buff;

      //append the DROPFILES structure
      buff.assign((const char *)&df, sizeof(df));

      //append each files
      for(size_t i=0; i<iFiles.size(); i++)
      {
        const std::string & utf8_file_path = iFiles[i];

        //Convert utf8 to unicode
        std::wstring unicode_file_path = utf8_to_unicode(utf8_file_path);

        //append
        const char * data_buffer = (const char*)unicode_file_path.c_str();
        const size_t data_size_bytes = (unicode_file_path.size() + 1) * sizeof(unicode_file_path[0]); // +1 for including the NULL terminating character
        buff.append(data_buffer, data_size_bytes);
      }
        
      //Append final empty filepath
      {
        //Convert utf8 to unicode
        std::wstring unicode_file_path = utf8_to_unicode("");

        //append
        const char * data_buffer = (const char*)unicode_file_path.c_str();
        const size_t data_size_bytes = (unicode_file_path.size() + 1) * sizeof(unicode_file_path[0]); // +1 for including the NULL terminating character
        buff.append(data_buffer, data_size_bytes);
      }

      //copy data to global allocated memory
      HGLOBAL hMem = GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE, buff.size());
      if (hMem == NULL)
        return false;
      void * buffer = GlobalLock(hMem);
      memcpy(buffer, buff.c_str(), buff.size());
      GlobalUnlock(hMem);

      //put it on the clipboard
      HANDLE hData = SetClipboardData( CF_HDROP, hMem );
      if (hData != hMem)
        return false;
    }

    //Register iDragDropType
    {
      HGLOBAL hDropEffect = GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE,sizeof(DWORD));
      DWORD * dwDropEffect = (DWORD*)GlobalLock(hDropEffect);
      if (iDragDropType == Clipboard::DragDropCopy)
        (*dwDropEffect) = DROPEFFECT_COPY /*| DROPEFFECT_LINK*/;
      else
        (*dwDropEffect) = DROPEFFECT_MOVE;
      GlobalUnlock(hDropEffect);

      //put it on the clipboard
      HANDLE hData = SetClipboardData( gFormatDescriptorDropEffect, hDropEffect );
      if (hData != hDropEffect)
        return false;
    }

    return true;
  }

  bool Clipboard::getAsDragDropFiles(DragDropType & oDragDropType, Clipboard::StringVector & oFiles)
  {
    //Invalidate
    oDragDropType = Clipboard::DragDropType(-1);
    oFiles.clear();

    ClipboardObject obj( DEFAULT_READ_CLIPBOARD_HANDLE );
    if (!obj.isOpened())
      return false;

    //Detect if CUT or COPY
    {
      HANDLE hDropEffect = ::GetClipboardData (gFormatDescriptorDropEffect);
      if (hDropEffect)
      {
        LPVOID lpResults = GlobalLock(hDropEffect);
        {
          SIZE_T lBufferSize = GlobalSize(lpResults);
          DWORD dropEffect = *((DWORD*)lpResults);
          //if (dropEffect == DROPEFFECT_NONE)
          //  "NONE:"
          if (dropEffect & DROPEFFECT_COPY)
            oDragDropType = DragDropCopy;
          else if (dropEffect & DROPEFFECT_MOVE)
            oDragDropType = DragDropCut;
          //if (dropEffect & DROPEFFECT_LINK)
          //  "LINK:"
          //if (dropEffect & DROPEFFECT_SCROLL)
          //  "SCROLL:"
        }
        GlobalUnlock(hDropEffect);
      }

      if (oDragDropType == -1)
      {
        //unknown drop effect
        return false;
      }
    }

    //Retreive files
    HDROP hDrop = (HDROP) ::GetClipboardData (CF_HDROP);
    if (hDrop == NULL)
      return false;

    LPVOID lpResults = GlobalLock(hDrop);
    if (hDrop == NULL)
      return false;

    SIZE_T lBufferSize = GlobalSize(lpResults);
              
    // Find out how many file names the HDROP contains.
    int nCount = ::DragQueryFile (hDrop, (UINT) -1, NULL, 0);
    if (nCount == 0)
      return false;

    //Find out if files are unicode or ansi
    DROPFILES df = {0};
    df = *( ((DROPFILES*)lpResults) );
    bool isUnicode = (df.fWide == 1);

    // Enumerate the file names.
    for (int i=0; i<nCount; i++)
    {
      std::string file_path;

      if (isUnicode)
      {
        WCHAR szFile[MAX_PATH];
        UINT length = ::DragQueryFileW (hDrop, i, szFile, sizeof (szFile) / sizeof (WCHAR));
                  
        std::wstring utf16_file_path = szFile;

        //Convert from unicode to ansi
        file_path = unicode_to_ansi( utf16_file_path );
      }
      else
      {
        char szFile[MAX_PATH];
        ::DragQueryFileA (hDrop, i, szFile, sizeof (szFile) / sizeof (char));
        file_path = szFile;
      }

      //Add to output files
      //printf("Reading from clipboard %02d/%02d: %s\n", i+1, nCount, filePath.c_str());
      oFiles.push_back(file_path);
    }

    GlobalUnlock(hDrop);
          
    return true;
  }
} //namespace win32clipboard
