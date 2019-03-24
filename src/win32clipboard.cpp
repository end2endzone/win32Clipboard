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

namespace win32clipboard
{
  static UINT gFormatDescriptorBinary     = RegisterClipboardFormat("WIN32CLIPBOARD_BINARY_FORMAT");
  static UINT gFormatDescriptorDropEffect = RegisterClipboardFormat("Preferred DropEffect");

  static const std::string CRLF = ra::environment::getLineSeparator();
  static const std::string  EMPTY_STRING;
  static const std::wstring EMPTY_WIDE_STRING;

  #define DEFAULT_READ_CLIPBOARD_HANDLE   NULL
  #define DEFAULT_WRITE_CLIPBOARD_HANDLE  GetDesktopWindow()

  // Convert a wide Unicode string to an UTF8 string
  std::string unicode_to_utf8(const std::wstring & wstr)
  {
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
	  int num_characters = MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), NULL, 0);
    if (num_characters == 0)
      return EMPTY_WIDE_STRING;
	  std::wstring wstrTo(num_characters, 0);
	  MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), &wstrTo[0], num_characters);
	  return wstrTo;
  }

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

  void Clipboard::empty()
  {
    BOOL openSuccess = OpenClipboard( DEFAULT_WRITE_CLIPBOARD_HANDLE ); 
    BOOL emptySuccess = EmptyClipboard();
    BOOL closeSuccess = CloseClipboard(); 
  }

  bool Clipboard::isEmpty()
  {
    bool empty = true;

    //Check if the clipboard contains any of the formats available
    for(size_t i=0; i<Clipboard::NUM_FORMATS; i++)
    {
      Clipboard::Format test_format = (Clipboard::Format)i;
      if (contains(test_format))
      {
        empty = false;
      }
    }
    return empty;
  }

  bool Clipboard::contains(Clipboard::Format iClipboardFormat)
  {
    bool containsFormat = false;
    if ( OpenClipboard( DEFAULT_READ_CLIPBOARD_HANDLE ) ) 
    {
      switch(iClipboardFormat)
      {
      case Clipboard::FormatText:
        {
          HANDLE hData = GetClipboardData(CF_TEXT);
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
      CloseClipboard();
    }
    return containsFormat;
  }

  void Clipboard::setText(const std::string & iText)
  {
    if ( OpenClipboard( DEFAULT_WRITE_CLIPBOARD_HANDLE ) ) 
    {
      //replace \n by windows CRLF, if required
      std::string text_copy = iText;
      bool have_windows_newline = (text_copy.find(CRLF) != std::string::npos);
      bool have_linux_newline = (text_copy.find("\n") != std::string::npos);
      if (!have_windows_newline && have_linux_newline)
      {
        //text is not CRLF fixed yet
        ra::strings::replace(text_copy, "\n", CRLF);
      }

      size_t memory_size = text_copy.size()+1; // +1 to include the NULL terminating character

      HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, memory_size);
      if (hData != INVALID_HANDLE_VALUE)
      {
        void * buffer = GlobalLock(hData);
        memcpy(buffer, text_copy.c_str(), memory_size);
        GlobalUnlock(hData);
        BOOL emptySuccess = EmptyClipboard();
        SetClipboardData(CF_TEXT, hData);
      }
      CloseClipboard();
    }
  }

  bool Clipboard::getAsText(std::string & oText)
  {
    bool success = false;
    if (contains(Clipboard::FormatText))
    {
      if ( OpenClipboard( DEFAULT_READ_CLIPBOARD_HANDLE ) ) 
      {
        HANDLE hData = GetClipboardData(CF_TEXT);
        void * buffer = GlobalLock( hData );
        oText = (char*)buffer;
        GlobalUnlock( hData );
        CloseClipboard();

        //Replace CRLF by \n so that it is c++ friendly
        ra::strings::replace(oText, CRLF, "\n");

        success = true;
      }
    }
    return success;
  }

  void Clipboard::setBinary(const MemoryBuffer & iMemoryBuffer)
  {
    if ( OpenClipboard( DEFAULT_WRITE_CLIPBOARD_HANDLE ) )
    {
      //allocate some global memory
      BOOL emptySuccess = EmptyClipboard();
      HGLOBAL hMem = GlobalAlloc(GMEM_DDESHARE, iMemoryBuffer.size());
      memcpy(GlobalLock(hMem), iMemoryBuffer.c_str(), iMemoryBuffer.size());
      GlobalUnlock(hMem);

      //Put it on the clipboard
      SetClipboardData(gFormatDescriptorBinary, hMem);

      CloseClipboard();
    }
  }

  bool Clipboard::getAsBinary(MemoryBuffer & oMemoryBuffer)
  {
    bool success = false;
    if (contains(Clipboard::FormatBinary))
    {
      if ( OpenClipboard( DEFAULT_READ_CLIPBOARD_HANDLE ) )
      {
        //get the buffer
        HANDLE hData = GetClipboardData(gFormatDescriptorBinary);
        size_t data_size = (size_t)GlobalSize(hData);
        char * buffer = (char*)GlobalLock( hData );

        //make a local copy
        oMemoryBuffer.assign(buffer, data_size);

        GlobalUnlock( hData );
        CloseClipboard();

        success = true;
      }
    }
    return success;
  }

  bool Clipboard::setDragDropFiles(const Clipboard::DragDropType & iDragDropType, const Clipboard::StringVector & iFiles)
  {
    //http://support.microsoft.com/kb/231721/en-us
    //http://aclacl.brinkster.net/MFC/ch19b.htm
    //http://read.pudn.com/downloads22/sourcecode/windows/multimedia/73340/ShitClass3/%E7%B1%BB%E5%8C%85/SManageFile.cpp__.htm

    //Validate drag drop type
    if (iDragDropType == Clipboard::DragDropCopy ||
        iDragDropType == Clipboard::DragDropCut)
    {
    }
    else
      return false;

    bool success = false;

    //Process with setting clipboard
    if ( OpenClipboard( DEFAULT_WRITE_CLIPBOARD_HANDLE ) ) 
    {
      BOOL emptySuccess = EmptyClipboard();

      //Register iFiles
      bool registerFilesSuccess = false;
      {
        DROPFILES dropFiles = {0};
        dropFiles.pFiles = sizeof(DROPFILES);
        dropFiles.pt.x = 0;
        dropFiles.pt.y = 0;
        dropFiles.fNC = FALSE;
        dropFiles.fWide = TRUE;

        //Build the buffer content
        MemoryBuffer buff;

        //append dropFiles
        buff.assign((const char *)&dropFiles, sizeof(dropFiles));

        //append each files
        for(size_t i=0; i<iFiles.size(); i++)
        {
          const std::string & utf8_file_path = iFiles[i];
          const size_t utf8_file_path_bytes = utf8_file_path.size();

          //Convert utf8 to unicode
          std::wstring utf16_file_path = utf8_to_unicode(utf8_file_path);

          //append
          const char * data_buffer = (const char*)utf16_file_path.c_str();
          const size_t data_size_bytes = (utf16_file_path.size() + 1) * sizeof(utf16_file_path[0]); // +1 for including the NULL terminating character
          buff.append(data_buffer, data_size_bytes);
        }
        
        //Append final empty filepath
        {
          //Convert utf8 to unicode
          std::wstring utf16_file_path = utf8_to_unicode("");

          //append
          const char * data_buffer = (const char*)utf16_file_path.c_str();
          const size_t data_size_bytes = (utf16_file_path.size() + 1) * sizeof(utf16_file_path[0]); // +1 for including the NULL terminating character
          buff.append(data_buffer, data_size_bytes);
        }

        //DragDropCopy local buffer to a global memory buffer
        HGLOBAL hGblFiles = GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE, buff.size());
        LPVOID szData = GlobalLock(hGblFiles);
        memcpy(szData, buff.c_str(), buff.size());
        GlobalUnlock(hGblFiles);

        HANDLE hResult = SetClipboardData( CF_HDROP, hGblFiles );
        registerFilesSuccess = (hResult == hGblFiles);
          
        //std::string err = extSystem::getLastErrorMessage();
        //const char * err2 = err.c_str();
        //int a = 0;
      }

      //Register iDragDropType
      bool registerTypeSuccess = false;
      {
        HGLOBAL hDropEffect = GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE,sizeof(DWORD));
        DWORD * dwDropEffect = (DWORD*)GlobalLock(hDropEffect);
        if (iDragDropType == Clipboard::DragDropCopy)
          (*dwDropEffect) = DROPEFFECT_COPY /*| DROPEFFECT_LINK*/;
        else
          (*dwDropEffect) = DROPEFFECT_MOVE;
        GlobalUnlock(hDropEffect);

        HANDLE hResult = SetClipboardData(gFormatDescriptorDropEffect, hDropEffect);
        registerTypeSuccess = (hResult == hDropEffect);
          
        //extString err = extSystem::getLastErrorMessage();
        //const char * err2 = err.c_str();
        //int a = 0;
      }

      success = (registerFilesSuccess && registerTypeSuccess);

      CloseClipboard();
    }

    return success;
  }

  bool Clipboard::getAsDragDropFiles(DragDropType & oDragDropType, Clipboard::StringVector & oFiles)
  {
    bool success = false;
    //Invalidate
    oDragDropType = Clipboard::DragDropType(-1);
    oFiles.clear();

    //if (contains(Clipboard::Format::FormatText))
    {
      if ( OpenClipboard( DEFAULT_READ_CLIPBOARD_HANDLE ) )
      {
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
              if (dropEffect & DROPEFFECT_MOVE)
                oDragDropType = DragDropCut;
              //if (dropEffect & DROPEFFECT_LINK)
              //  "LINK:"
              //if (dropEffect & DROPEFFECT_SCROLL)
              //  "SCROLL:"
            }
            GlobalUnlock(hDropEffect);
          }
        }

        //Retreive files
        HDROP hDrop = (HDROP) ::GetClipboardData (CF_HDROP);
        if (hDrop != NULL)
        {
          LPVOID lpResults = GlobalLock(hDrop);
          if (lpResults)
          {
            SIZE_T lBufferSize = GlobalSize(lpResults);
              
            // Find out how many file names the HDROP contains.
            int nCount = ::DragQueryFile (hDrop, (UINT) -1, NULL, 0);

            if (nCount)
            {
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
            }

            GlobalUnlock(hDrop);

            success = (oDragDropType != -1) && (oFiles.size() > 0);
          }
        }
          
        CloseClipboard();
      } 
    }
    return success;
  }
} //namespace win32clipboard
