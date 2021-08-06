# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(BOARD myesp32)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(proj)

include(lib/CMakeLists.txt)

FILE(GLOB app_sources_c src/*.c)
FILE(GLOB app_sources_cpp src/*.cpp)

set(app_sources
  ${app_sources_c}
  ${app_sources_cpp}
)

target_sources(app PRIVATE ${app_sources} ${lib_sources})
