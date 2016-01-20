cmake_minimum_required(VERSION 3.0)
project(jmespath.cpp)

set(JMESPATH_PROJECT_NAME jmespath.cpp)
set(JMESPATH_TARGET_NAME jmespath)
if ((${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        OR (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        OR (${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang"))
    #std=c++1y - use c++14 language standard
    set(CMAKE_CXX_FLAGS "-std=c++1y")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    #wd4348 - ingore warning C4348 (A template parameter was redefined.)
    #EHsc - set synchronous exception handling
    set(CMAKE_CXX_FLAGS "/EHsc /wd4348")
endif()

find_package(Boost 1.57 REQUIRED)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(third_party)

add_library(${JMESPATH_TARGET_NAME} ${JMESPATH_SOURCE_FILES})

target_include_directories(${JMESPATH_TARGET_NAME}
    PUBLIC ${JMESPATH_INCLUDE_DIRECTORIES} ${Boost_INCLUDE_DIRS})
target_link_libraries(${JMESPATH_TARGET_NAME}
    PUBLIC ${Boost_LIBRARIES})
set_target_properties(${JMESPATH_TARGET_NAME} PROPERTIES
    COMPILE_DEFINITIONS "BOOST_SPIRIT_UNICODE=1")
