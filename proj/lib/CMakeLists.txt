if("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_LIST_DIR}")
  message("Creating Tests")
  cmake_minimum_required(VERSION 3.13.1)

  enable_language(CXX)
  set(CMAKE_CXX_STANDARD 14)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.8.1.zip
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
  enable_testing()

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

  FILE(GLOB lib_tests_c tests/*.c)
  FILE(GLOB lib_tests_cpp tests/*.cpp)
  FILE(GLOB lib_tests_h tests/*.h)
  set(lib_tests
    ${lib_tests_c}
    ${lib_tests_cpp}
    ${lib_tests_h}
  )
  
  add_executable(lib_tests ${lib_sources} ${lib_tests})
  target_link_libraries(lib_tests gtest_main)
  target_include_directories(lib_tests PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src" "${CMAKE_CURRENT_LIST_DIR}/tests")

  include(GoogleTest)
  gtest_discover_tests(lib_tests)

endif()
