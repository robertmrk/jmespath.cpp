cmake_minimum_required(VERSION 3.0)

set(JMESPATH_UNITTEST_TARGET_NAME unit)

add_executable(${JMESPATH_UNITTEST_TARGET_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/unit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/grammar_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/search_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser_test.cpp)

target_link_libraries(${JMESPATH_UNITTEST_TARGET_NAME} ${JMESPATH_TARGET_NAME})
