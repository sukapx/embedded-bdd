# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.13.1)

set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(BOARD mystm32)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(proj)





include(../common/src/CMakeLists.txt)

FILE(GLOB app_sources_c src/*.c)
FILE(GLOB app_sources_cpp src/*.cpp)

set(app_sources
  ${app_sources_c}
  ${app_sources_cpp}
)

target_sources(app PRIVATE ${app_sources})
target_link_libraries(app PUBLIC controllib)
