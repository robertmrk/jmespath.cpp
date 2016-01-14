cmake_minimum_required(VERSION 3.0)
project(jmespath.cpp)

set(JMESPATH_PROJECT_NAME jmespath.cpp)
set(JMESPATH_TARGET_NAME jmespath)
set(CMAKE_CXX_FLAGS "-std=c++1y -stdlib=libc++")

find_package(Boost 1.55 REQUIRED)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(third_party)

add_library(${JMESPATH_TARGET_NAME} ${JMESPATH_SOURCE_FILES})

set_target_properties(${JMESPATH_TARGET_NAME} PROPERTIES
    LINKER_LANGUAGE CXX
    COMPILE_FLAGS "-std=c++1y -stdlib=libc++")
target_include_directories(${JMESPATH_TARGET_NAME}
    PUBLIC ${JMESPATH_INCLUDE_DIRECTORIES} ${Boost_INCLUDE_DIRS})
target_link_libraries(${JMESPATH_TARGET_NAME}
    PUBLIC ${Boost_LIBRARIES})
