if("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_LIST_DIR}")
  message("Creating Tests")
  cmake_minimum_required(VERSION 3.13.1)
  project(proj)
endif()

FILE(GLOB lib_sources_c src/*.c)
FILE(GLOB lib_sources_cpp src/*.cpp)
FILE(GLOB lib_sources_h src/*.h)
set(lib_sources
  ${lib_sources_c}
  ${lib_sources_cpp}
  ${lib_sources_h}
)

if("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_LIST_DIR}")
  add_executable(tests ${lib_sources})
endif()
