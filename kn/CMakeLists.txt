cmake_minimum_required (VERSION 3.15)

project(kn)
find_package(GFLAGS)
find_package(Boost)
find_package(FOLLY)
find_package(date)
#find_package(prometheus-cpp)

set (CMAKE_VERBOSE_MAKEFILE ON)
set (CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -Wall -g -ggdb")
set (CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -Wall -O2")
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_REQUIRED_FLAGS -std=c++20)

message(${PROJECT_SOURCE_DIR})

include_directories(".")

#include_directories("../3rd/xxHash")
#aux_source_directory(../3rd/xxHash XXHASH_SRCS)
#include_directories("../3rd/date/include")
#file(GLOB DATE_SRCS "../3rd/date/src/*.cpp")

add_subdirectory(def)
add_subdirectory(log)
add_subdirectory(tool)
add_subdirectory(net)
add_subdirectory(etcd)
