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

include(src/CMakeLists.txt)


FILE(GLOB common_tests_c tests/*.c)
FILE(GLOB common_tests_cpp tests/*.cpp)
FILE(GLOB common_tests_h tests/*.h)
set(common_tests
  ${common_tests_c}
  ${common_tests_cpp}
  ${common_tests_h}
)

add_executable(common_tests ${common_tests})
target_include_directories(common_tests PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/tests"
  )

target_link_libraries(common_tests 
    gtest_main
    controllib
  )

include(GoogleTest)
gtest_discover_tests(common_tests)



FILE(GLOB common_shared_c sharedlib/*.c)
FILE(GLOB common_shared_cpp sharedlib/*.cpp)
FILE(GLOB common_shared_h sharedlib/*.h)
set(common_shared
  ${common_shared_c}
  ${common_shared_cpp}
  ${common_shared_h}
)

add_library(common_shared SHARED ${common_shared})
target_include_directories(common_shared PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/sharedlib"
  )

target_link_libraries(common_shared 
    controllib
  )