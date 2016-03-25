cmake_minimum_required(VERSION 3.0)
project(jmespath.cpp)

set(JMESPATH_COVERAGE_INFO OFF CACHE BOOL "Generate code coverage information")
set(JMESPATH_PROJECT_NAME jmespath.cpp)
set(JMESPATH_TARGET_NAME jmespath)
if ((${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        OR (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        OR (${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang"))
    #std=c++1y - use c++14 language standard
    set(CMAKE_CXX_FLAGS "-std=c++1y \
        -Weverything -Wno-padded -Wno-c++98-compat-pedantic \
        -Wno-disabled-macro-expansion \
        --system-header-prefix=boost --system-header-prefix=json.hpp")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    #wd4348 - ingore warning C4348 (A template parameter was redefined.)
    #EHsc - set synchronous exception handling
    #bigobj - fix for error C1128 (increase the number of sections in .obj file)
    set(CMAKE_CXX_FLAGS "/EHsc /wd4348 /bigobj")
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
target_compile_definitions(${JMESPATH_TARGET_NAME}
    PUBLIC "BOOST_SPIRIT_UNICODE=1")
if (${JMESPATH_COVERAGE_INFO})
    set_target_properties(${JMESPATH_TARGET_NAME} PROPERTIES
        COMPILE_FLAGS "--coverage"
        LINK_FLAGS "--coverage")
endif ()
