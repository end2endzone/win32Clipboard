find_package(GTest)
find_package(rapidassist)

add_executable(win32clipboard_unittest
  ${WIN32CLIPBOARD_EXPORT_HEADER}
  ${WIN32CLIPBOARD_VERSION_HEADER}
  ${WIN32CLIPBOARD_CONFIG_HEADER}
  main.cpp
  TestEncodingConversion.cpp
  TestEncodingConversion.h
  TestWin32Clipboard.cpp
  TestWin32Clipboard.h
)

# Unit test projects requires to link with pthread if also linking with gtest
if(NOT WIN32)
  set(PTHREAD_LIBRARIES -pthread)
endif()

# Force CMAKE_DEBUG_POSTFIX for executables
set_target_properties(win32clipboard_unittest PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

target_include_directories(win32clipboard_unittest PRIVATE ${GTEST_INCLUDE_DIR} rapidassist)
add_dependencies(win32clipboard_unittest win32clipboard)
target_link_libraries(win32clipboard_unittest PUBLIC win32clipboard PRIVATE ${PTHREAD_LIBRARIES} ${GTEST_LIBRARIES} rapidassist )

install(TARGETS win32clipboard_unittest
        EXPORT win32clipboard-targets
        ARCHIVE DESTINATION ${WIN32CLIPBOARD_INSTALL_LIB_DIR}
        LIBRARY DESTINATION ${WIN32CLIPBOARD_INSTALL_LIB_DIR}
        RUNTIME DESTINATION ${WIN32CLIPBOARD_INSTALL_BIN_DIR}
)
