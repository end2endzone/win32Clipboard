find_package(rapidassist REQUIRED)

set(WIN32CLIPBOARD_HEADER_FILES ""
  ${CMAKE_SOURCE_DIR}/include/win32clipboard/win32clipboard.h
)

add_library(win32clipboard STATIC
  ${WIN32CLIPBOARD_HEADER_FILES}
  ${WIN32CLIPBOARD_EXPORT_HEADER}
  ${WIN32CLIPBOARD_VERSION_HEADER}
  ${WIN32CLIPBOARD_CONFIG_HEADER}
  win32clipboard.cpp
)

# Unit test projects requires to link with pthread if also linking with gtest
if(WIN32CLIPBOARD_BUILD_GTESTHELP)
  if(NOT WIN32)
    set(PTHREAD_LIBRARIES -pthread)
  endif()
endif()

# Force CMAKE_DEBUG_POSTFIX for executables
set_target_properties(win32clipboard PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

# Define include directories for exported code.
target_include_directories(win32clipboard
  PUBLIC
    $<INSTALL_INTERFACE:${WIN32CLIPBOARD_INSTALL_INCLUDE_DIR}>  # for clients using the installed library.
  PRIVATE
    ${GTEST_INCLUDE_DIR}
    rapidassist
)
target_link_libraries(win32clipboard PRIVATE ${PTHREAD_LIBRARIES} ${GTEST_LIBRARIES} rapidassist)

install(TARGETS win32clipboard
        EXPORT win32clipboard-targets
        ARCHIVE DESTINATION ${WIN32CLIPBOARD_INSTALL_LIB_DIR}
        LIBRARY DESTINATION ${WIN32CLIPBOARD_INSTALL_LIB_DIR}
        RUNTIME DESTINATION ${WIN32CLIPBOARD_INSTALL_BIN_DIR}
)
