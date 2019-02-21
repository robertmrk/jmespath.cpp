cmake_minimum_required(VERSION 3.0)
project(example)

find_package(jmespath 0.2.0 CONFIG REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} jmespath::jmespath)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)
