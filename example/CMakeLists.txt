cmake_minimum_required(VERSION 3.0)
project(example)

find_package(jmespath 0.1.0 CONFIG REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} robertmrk::jmespath)
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON)
