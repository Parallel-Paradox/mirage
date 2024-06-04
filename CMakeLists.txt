cmake_minimum_required(VERSION 3.29)
project(mirage_engine)

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
  add_compile_options(/W4)
else ()
  add_compile_options(-Wall -Wextra -pedantic)
endif ()

# --- Build mirage engine ---

option(MIRAGE_BUILD_SHARED "Build shared mirage engine" ON)

set(SRC
    src/mirage_framework/application.cpp)
if (MIRAGE_BUILD_SHARED)
  message("Building shared mirage engine...")
  add_library(mirage_engine SHARED ${SRC})
  target_compile_definitions(mirage_engine PUBLIC MIRAGE_BUILD_SHARED)
else ()
  message("Building static mirage engine...")
  add_library(mirage_engine STATIC ${SRC})
endif ()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
  target_compile_definitions(mirage_engine PUBLIC DEBUG)
endif ()
target_compile_definitions(mirage_engine PUBLIC MIRAGE_BUILD)

target_include_directories(mirage_engine PUBLIC src)

# Build examples and tests if library standalone
string(COMPARE EQUAL
    "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}" MIRAGE_STANDALONE)
if (MIRAGE_STANDALONE)
  add_subdirectory(test)
endif ()
