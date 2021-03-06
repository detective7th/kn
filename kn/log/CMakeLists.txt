cmake_minimum_required (VERSION 3.15)

project(log)
#find_package(g3logger)

set (CMAKE_VERBOSE_MAKEFILE ON)
set (CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -Wall -g -ggdb")
set (CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -Wall -O2")
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_REQUIRED_FLAGS -std=c++20)

set(TARGET log)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_REVISION 1)
set(VERSION_BUILD 0)

message(${PROJECT_SOURCE_DIR})
message(${CMAKE_CURRENT_BINARY_DIR})
message(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_INSTALL_PREFIX "/usr/local")

set(INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/lib)
set(INSTALL_INC_DIR ${CMAKE_INSTALL_PREFIX}/include/kn/log)

set(SHARED_LIBS ${SHARED_LIBS} g3logger g3logrotate)

if (CMAKE_BUILD_TYPE STREQUAL Debug OR (CMAKE_BUILD_TYPE STREQUAL DEBUG))
  add_definitions(-DDDEBUG)
endif()

configure_file(
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_SOURCE_DIR}/src/config.h"
  )

file (GLOB HEAD_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
install (FILES ${HEAD_FILES} DESTINATION ${INSTALL_INC_DIR})
