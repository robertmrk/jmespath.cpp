cmake_minimum_required(VERSION 3.0)

add_library(Json INTERFACE IMPORTED GLOBAL)
set_target_properties(Json PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/json/src)

add_library(FakeIt INTERFACE IMPORTED GLOBAL)
set(FAKEIT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/FakeIt)
set_target_properties(FakeIt PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
    "${FAKEIT_DIR}/include;${FAKEIT_DIR}/config/catch")
