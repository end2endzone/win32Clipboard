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

#include "TestEncodingConversion.h"

#include "win32clipboard/win32clipboard.h"

#include "rapidassist/testing.h"

using namespace win32clipboard;

namespace win32clipboard { namespace test
{
  //--------------------------------------------------------------------------------------------------
  void TestEncodingConversion::SetUp()
  {
  }
  //--------------------------------------------------------------------------------------------------
  void TestEncodingConversion::TearDown()
  {
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestEncodingConversion, testAnsiUnicode)
  {
    const std::string str_ansi = "école"; //school in french

    std::wstring str_unicode = ansi_to_unicode(str_ansi);
    std::string str_converted = unicode_to_ansi(str_unicode);

    ASSERT_EQ( str_ansi.size(), str_unicode.size() );
    ASSERT_EQ( str_ansi, str_converted );
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestEncodingConversion, testUtf8Unicode)
  {
    const std::string str_utf8 = "\303\251cole"; //school in french

    std::wstring str_unicode = utf8_to_unicode(str_utf8);
    std::string str_converted = unicode_to_utf8(str_unicode);

    //The size() method returns the number of character, not the numbre of byte.
    //The utf8 string have one more character than the unicode version because the first characters is counted as 2 bytes because std::string is expected 1 byte/characters.
    ASSERT_EQ( str_utf8.size() - 1, str_unicode.size() );  
    
    //unicode string is missing a byte because the first utf8 characters is 2 bytes in utf but a single character in unicode.
    ASSERT_EQ( str_utf8, str_converted );
  }
  //--------------------------------------------------------------------------------------------------
 
} //namespace test
} //namespace win32clipboard
