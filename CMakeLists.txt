cmake_minimum_required (VERSION 3.10)

project (
  optionpp
  VERSION 2.0
  DESCRIPTION "C++ library for reading command-line options"
  )

set (OPTIONPP_NAME "Option++")
set (OPTIONPP_VERSION "${PROJECT_VERSION}")
set (OPTIONPP_DESCRIPTION "${PROJECT_DESCRIPTION}")
set (OPTIONPP_LIBDIR "optionpp-${OPTIONPP_VERSION}")

option (OPTIONPP_STATIC "Build static library instead of shared" OFF)
option (OPTIONPP_TEST "Build unit tests" ON)
option (OPTIONPP_DOCS "Generate documentation" ON)
option (OPTIONPP_EXAMPLES "Build examples" ON)

# Require standard C++11
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

# Export symbols on Windows
set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Create pkg-config file
configure_file(liboptionpp.pc.in "${CMAKE_CURRENT_BINARY_DIR}/liboptionpp.pc" @ONLY)

# See if Doxygen is available
if (OPTIONPP_DOCS)
  find_package (Doxygen)
  if (DOXYGEN_FOUND)
    # Generate Doxygen config file
    set (DOXYGEN_IN "${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in")
    set (DOXYGEN_OUT "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")

    configure_file ("${DOXYGEN_IN}" "${DOXYGEN_OUT}")

    add_custom_target (doc ALL
      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      VERBATIM
      )
  else () # No Doxygen
    message ("Doxygen not found, documentation will not be generated")
  endif ()
endif ()

set (OPTIONPP_SOURCE_FILES
  src/error.cpp
  src/option.cpp
  src/option_group.cpp
  src/parser.cpp
  src/parser_result.cpp
  src/result_iterator.cpp
  src/utility.cpp
  )

set (OPTIONPP_PUBLIC_HEADER_FILES
  include/optionpp/opt
  )

set (OPTIONPP_TEST_FILES
  test/tst_main.cpp
  test/tst_option.cpp
  test/tst_parser.cpp
  test/tst_parser_result.cpp
  test/tst_result_iterator.cpp
  test/tst_utility.cpp
  )

set (OPTIONPP_EXAMPLES
  docs/examples/basic.cpp
  docs/examples/dos.cpp
  docs/examples/from_string.cpp
  docs/examples/minimal.cpp
  docs/examples/mygrep.cpp
  docs/examples/tutorial.cpp
  )

if (OPTIONPP_STATIC)
  # Build static library
  add_library (optionpp STATIC "${OPTIONPP_SOURCE_FILES}")
  target_include_directories (optionpp PRIVATE include)
else ()
  # Build shared library
  add_library (optionpp SHARED "${OPTIONPP_SOURCE_FILES}")
  target_include_directories (optionpp PRIVATE include)
endif ()

if (OPTIONPP_TEST)
  # Build test executable
  add_executable (test "${OPTIONPP_TEST_FILES}")
  target_link_libraries (test PRIVATE optionpp)
  target_include_directories (test PRIVATE include third_party)
  add_test (NAME test COMMAND test)
endif ()

if (OPTIONPP_EXAMPLES)
  # Build examples
  foreach (example IN LISTS OPTIONPP_EXAMPLES)
    get_filename_component (CURRENT_EXAMPLE "${example}" NAME_WE)
    set (CURRENT_EXAMPLE "example_${CURRENT_EXAMPLE}")
    add_executable (
      ${CURRENT_EXAMPLE}
      "${example}"
      )
    target_include_directories (${CURRENT_EXAMPLE} PRIVATE single_header)
    target_compile_definitions (${CURRENT_EXAMPLE} PRIVATE OPTIONPP_MAIN)
  endforeach ()
endif ()

# Set max warning level
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(optionpp PRIVATE -Wall -Wextra -pedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_compile_options(optionpp PRIVATE /W4)
endif ()

# Installation
install (
  TARGETS optionpp
  EXPORT optionppConfig
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static
  )
install (
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
  DESTINATION include
  )
install (
  FILES "${CMAKE_BINARY_DIR}/liboptionpp.pc"
  DESTINATION share/pkgconfig
  )
export (
  TARGETS optionpp
  FILE "${CMAKE_CURRENT_BINARY_DIR}/optionppConfig.cmake"
  )
install (
  EXPORT optionppConfig
  DESTINATION share/optionpp/cmake
  )
