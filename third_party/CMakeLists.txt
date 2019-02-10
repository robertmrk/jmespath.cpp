cmake_minimum_required(VERSION 3.0)

add_library(FakeIt INTERFACE IMPORTED GLOBAL)
set(FAKEIT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/FakeIt)
set_target_properties(FakeIt PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
    "${FAKEIT_DIR}/include;${FAKEIT_DIR}/config/catch")

add_library(Catch2 INTERFACE IMPORTED GLOBAL)
set(CATCH2_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Catch2)
set_target_properties(Catch2 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
    "${CATCH2_DIR}/single_include/catch2")
