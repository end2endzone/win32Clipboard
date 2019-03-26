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

#include "TestWin32Clipboard.h"

#include "win32clipboard/win32clipboard.h"

#include "rapidassist/gtesthelp.h"
#include "rapidassist/filesystem.h"
#include "rapidassist/environment.h"
#include "rapidassist/time_.h"

using namespace win32clipboard;

namespace win32clipboard { namespace test
{
  //--------------------------------------------------------------------------------------------------
  void TestWin32Clipboard::SetUp()
  {
  }
  //--------------------------------------------------------------------------------------------------
  void TestWin32Clipboard::TearDown()
  {
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestWin32Clipboard, testSetGetText)
  {
    Clipboard & c = Clipboard::getInstance();

    static const char * values[] = {
      "hello world",
      "foo",
      "bar",
    };
    static const size_t num_values = sizeof(values) / sizeof(values[0]);

    c.empty();

    for(size_t i=0; i<num_values; i++)
    {
      const char * value = values[i];
      std::string str = value;

      bool status = c.setText(str);
      ASSERT_TRUE( status );
    
      std::string text;
      status = c.getAsText(text);
      ASSERT_TRUE( status );

      ASSERT_EQ( str, text ) << "Failed setting clipboard to value '" << str << "'. The returned value is '" << text << "'.";
    }
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestWin32Clipboard, testEmpty)
  {
    Clipboard & c = Clipboard::getInstance();

    static const std::string SAMPLE_TEXT = "empty";
    ASSERT_TRUE( c.setText(SAMPLE_TEXT) );

    ASSERT_TRUE( c.empty() );

    ASSERT_TRUE( c.isEmpty() );

    std::string text;
    bool status = c.getAsText(text);
    ASSERT_FALSE( status );
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestWin32Clipboard, testSetBinary)
  {
    Clipboard & c = Clipboard::getInstance();

    //initialize a binary buffer
    static const size_t BUFFER_SIZE = 1024;
    static unsigned char buffer[BUFFER_SIZE];
    for(size_t i=0; i<BUFFER_SIZE; i++)
    {
      buffer[i] = (char)(i%256);
    }

    Clipboard::MemoryBuffer input;
    input.assign((char*)buffer, BUFFER_SIZE);

    bool status = c.setBinary(input);
    ASSERT_TRUE( status );

    Clipboard::MemoryBuffer output;
    status = c.getAsBinary(output);
    ASSERT_TRUE( status );

    //both binary buffer are identical
    ASSERT_EQ( BUFFER_SIZE, output.size() );
    for(size_t i=0; i<BUFFER_SIZE; i++)
    {
      char c1 = buffer[i];
      char c2 = output[i];
      ASSERT_EQ( c1, c2 ) << "Failed using getAsBinary() at offset '" << i << "'. Expecting character " << (int)c1 << " (decimal) but got character " << (int)c2 << " (decimal).";
    }
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestWin32Clipboard, testUnixNewLine)
  {
    Clipboard & c = Clipboard::getInstance();

    static const std::string SAMPLE_TEXT = "1\n2\n3\n4\n5";
    c.setText(SAMPLE_TEXT);

    std::string text;
    bool status = c.getAsText(text);
    ASSERT_TRUE( status );

    ASSERT_EQ( SAMPLE_TEXT, text ) << "Failed setting clipboard to value '" << SAMPLE_TEXT << "'. The returned value is '" << text << "'.";
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestWin32Clipboard, testWindowsNewLine)
  {
    Clipboard & c = Clipboard::getInstance();

    static const std::string SAMPLE_TEXT = "1\r\n2\r\n3\r\n4\r\n5";
    c.setText(SAMPLE_TEXT);

    std::string text;
    bool status = c.getAsText(text);
    ASSERT_TRUE( status );

    ASSERT_EQ( SAMPLE_TEXT, text ) << "Failed setting clipboard to value '" << SAMPLE_TEXT << "'. The returned value is '" << text << "'.";
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestWin32Clipboard, testDragDropFiles)
  {
    Clipboard & c = Clipboard::getInstance();

    static const char * values[] = {
      "C:\\Windows\\System32\\notepad.exe",
      "C:\\Program Files (x86)\\Windows Media Player\\wmplayer.exe",
      "C:\\Windows\\System32\\write.exe",
    };
    static const size_t num_values = sizeof(values) / sizeof(values[0]);

    //define the list of files
    Clipboard::StringVector input_files;
    for(size_t i=0; i<num_values; i++)
    {
      const char * value = values[i];
      input_files.push_back(value);
    }

    //
    static const Clipboard::DragDropType input_type = Clipboard::DragDropCopy;
    bool status = c.setDragDropFiles(input_type, input_files);
    ASSERT_TRUE( status );

    Clipboard::DragDropType output_type;
    Clipboard::StringVector output_files;
    status = c.getAsDragDropFiles(output_type, output_files);
    ASSERT_TRUE( status );

    ASSERT_EQ( (int)input_type, (int)output_type );
    
    //validate the files
    ASSERT_EQ( input_files.size(), output_files.size() );
    for(size_t i=0; i<input_files.size(); i++)
    {
      const std::string &  input_file =  input_files[i];
      const std::string & output_file = output_files[i];
      ASSERT_EQ( input_file, output_file );
    }
  }
  //--------------------------------------------------------------------------------------------------
 
} //namespace test
} //namespace win32clipboard
