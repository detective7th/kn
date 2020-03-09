cmake_minimum_required (VERSION 3.15)

project(net)
#find_package(g3logger)
find_package(Boost)
find_package(FOLLY)

set (CMAKE_VERBOSE_MAKEFILE ON)
set (CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -Wall -g -ggdb")
set (CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -Wall -O2 -Wall")
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_REQUIRED_FLAGS -std=c++20)

set(TARGET net)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_REVISION 1)
set(VERSION_BUILD 0)

message(${PROJECT_SOURCE_DIR})
message(${CMAKE_CURRENT_BINARY_DIR})
message(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_INSTALL_PREFIX "/usr/local")

set(INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/lib)
set(INSTALL_INC_DIR ${CMAKE_INSTALL_PREFIX}/include/kn/net)

set(SHARED_LIBS ${SHARED_LIBS} boost_coroutine g3logger g3logrotate)
set (STATIC_LIBS ${STATIC_LIBS}
  folly
  fizz
  mvfst_exception
  mvfst_codec
  mvfst_codec_decode
  mvfst_codec_types
  mvfst_codec_pktbuilder
  mvfst_codec_pktrebuilder
  mvfst_codec_packet_number_cipher
  mvfst_cc_algo
  mvfst_transport
  mvfst_flowcontrol
  mvfst_state_functions
  mvfst_state_machine
  mvfst_state_stream
  mvfst_state_stream_functions
  mvfst_looper
  mvfst_handshake
  mvfst_state_pacing_functions
  mvfst_state_ack_handler
  mvfst_state_qpr_functions
  mvfst_state_simple_frame_functions
  mvfst_loss
  mvfst_qlogger
  mvfst_happyeyeballs
  mvfst_server)

if (CMAKE_BUILD_TYPE STREQUAL Debug OR (CMAKE_BUILD_TYPE STREQUAL DEBUG))
  add_definitions(-DDDEBUG)
endif()

configure_file(
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_SOURCE_DIR}/src/config.h"
  )

add_definitions(-DBOOST_COROUTINES_NO_DEPRECATION_WARNING)

#file (GLOB HEAD_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
#install (FILES ${HEAD_FILES} DESTINATION ${INSTALL_INC_DIR})
install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/udp DESTINATION ${INSTALL_INC_DIR} FILES_MATCHING PATTERN "*.h")
install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/quic DESTINATION ${INSTALL_INC_DIR} FILES_MATCHING PATTERN "*.h")
install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/ws DESTINATION ${INSTALL_INC_DIR} FILES_MATCHING PATTERN "*.h")
