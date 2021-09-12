
FILE(GLOB lib_sources_c ${CMAKE_CURRENT_LIST_DIR}/*.c)
FILE(GLOB lib_sources_cpp ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
FILE(GLOB lib_sources_h ${CMAKE_CURRENT_LIST_DIR}/*.h)

set(lib_sources
  ${lib_sources_c}
  ${lib_sources_cpp}
  ${lib_sources_h}
)


add_library(controllib STATIC
    ${lib_sources}
)
set_target_properties(controllib PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_LIST_DIR})
