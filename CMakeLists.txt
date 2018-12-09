cmake_minimum_required(VERSION 3.0)
project(jmespath.cpp VERSION 0.1.0)

set(JMESPATH_CONAN_BUILD_INFO "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
if (EXISTS ${JMESPATH_CONAN_BUILD_INFO})
    include(${JMESPATH_CONAN_BUILD_INFO})
    conan_basic_setup()
endif()

set(JMESPATH_COVERAGE_INFO OFF CACHE BOOL "Generate code coverage information")
set(JMESPATH_PROJECT_NAME ${PROJECT_NAME})
set(JMESPATH_TARGET_NAME "jmespath")
set(JMESPATH_COMPILE_FLAGS " ")
set(JMESPATH_INCLUDE_DIR "include")
set(JMESPATH_LIB_DIR "lib")
set(JMESPATH_INCLUDE_PREFIX_DIR "${JMESPATH_INCLUDE_DIR}/jmespath")
if ((${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        OR (${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang"))
    set(JMESPATH_COMPILE_FLAGS "-Weverything -Wno-padded \
        -Wno-c++98-compat-pedantic -Wno-disabled-macro-expansion \
        -Wno-weak-vtables")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    #wd4348 - ingore warning C4348 (A template parameter was redefined.)
    #wd4068 - ingore warning C4068 (The compiler ignored an unrecognized pragma)
    #EHsc - set synchronous exception handling
    #bigobj - fix for error C1128 (increase the number of sections in .obj file)
    set(JMESPATH_COMPILE_FLAGS "/EHsc /wd4348 /wd4068 /bigobj")
endif()
enable_testing()

find_package(Boost 1.57 REQUIRED)
list(APPEND JMESPATH_SYSTEM_INCLUDE_DIRECTORIES
    ${Boost_INCLUDE_DIRS})

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(third_party)

add_library(${JMESPATH_TARGET_NAME} ${JMESPATH_SOURCE_FILES})

add_dependencies(${JMESPATH_TARGET_NAME}
    ${JMESPATH_SYMLINK_THIRD_PARTY_TARGET_NAME})
target_include_directories(${JMESPATH_TARGET_NAME}
    PUBLIC ${JMESPATH_INCLUDE_DIRECTORIES})
target_include_directories(${JMESPATH_TARGET_NAME} SYSTEM
    PUBLIC ${JMESPATH_SYSTEM_INCLUDE_DIRECTORIES})
set_target_properties(${JMESPATH_TARGET_NAME} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    COMPILE_FLAGS ${JMESPATH_COMPILE_FLAGS}
    DEBUG_POSTFIX "d"
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
target_link_libraries(${JMESPATH_TARGET_NAME}
    PUBLIC ${Boost_LIBRARIES})
target_compile_definitions(${JMESPATH_TARGET_NAME}
    PUBLIC "BOOST_SPIRIT_UNICODE=1")
if (${JMESPATH_COVERAGE_INFO})
    set_target_properties(${JMESPATH_TARGET_NAME} PROPERTIES
        COMPILE_FLAGS "-fprofile-arcs  -ftest-coverage"
        LINK_FLAGS "-fprofile-arcs  -ftest-coverage -lgcov")
endif ()

include(CMakePackageConfigHelpers)
SET(JMESPATH_PACKAGE_NAME ${JMESPATH_TARGET_NAME})
SET(JMESPATH_CONFIG_INSTALL_DIR
    "${JMESPATH_LIB_DIR}/cmake/${JMESPATH_PACKAGE_NAME}")
SET(JMESPATH_CONFIG_FILE_NAME
    "${JMESPATH_PACKAGE_NAME}Config.cmake")
SET(JMESPATH_CONFIGVERSION_FILE_NAME
    "${JMESPATH_PACKAGE_NAME}ConfigVersion.cmake")
SET(JMESPATH_TARGETS_FILE_NAME "${JMESPATH_PACKAGE_NAME}Targets.cmake")

configure_package_config_file("cmake/config.cmake.in"
    "${CMAKE_BINARY_DIR}/${JMESPATH_CONFIG_FILE_NAME}"
    INSTALL_DESTINATION ${JMESPATH_CONFIG_INSTALL_DIR})
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${JMESPATH_CONFIGVERSION_FILE_NAME}"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(TARGETS ${JMESPATH_TARGET_NAME}
    EXPORT ${JMESPATH_PACKAGE_NAME}
    ARCHIVE DESTINATION lib)

export(TARGETS ${JMESPATH_TARGET_NAME}
    FILE "${CMAKE_BINARY_DIR}/${JMESPATH_TARGETS_FILE_NAME}")
export(PACKAGE ${JMESPATH_PACKAGE_NAME})

install(EXPORT ${JMESPATH_PACKAGE_NAME}
    FILE ${JMESPATH_TARGETS_FILE_NAME}
    DESTINATION ${JMESPATH_CONFIG_INSTALL_DIR})

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${JMESPATH_CONFIG_FILE_NAME}"
    "${CMAKE_CURRENT_BINARY_DIR}/${JMESPATH_CONFIGVERSION_FILE_NAME}"
    DESTINATION ${JMESPATH_CONFIG_INSTALL_DIR})

install(FILES "src/jmespath/jmespath.h"
    "src/jmespath/expression.h"
    "src/jmespath/types.h"
    "src/jmespath/exceptions.h"
    DESTINATION ${JMESPATH_INCLUDE_PREFIX_DIR})
