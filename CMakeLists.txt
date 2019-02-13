cmake_minimum_required(VERSION 3.8)
project(jmespath.cpp VERSION 0.1.0 LANGUAGES CXX)

set(JMESPATH_CONAN_BUILD_INFO "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
if (EXISTS ${JMESPATH_CONAN_BUILD_INFO})
    include(${JMESPATH_CONAN_BUILD_INFO})
    conan_basic_setup()
endif()

##
## CONFIGURATION
##
option(JMESPATH_COVERAGE_INFO "Generate code coverage information" OFF)
set(JMESPATH_PROJECT_NAME ${PROJECT_NAME})
set(JMESPATH_TARGET_NAME "jmespath")
SET(JMESPATH_TARGET_NAMESPACE_NAME "${JMESPATH_TARGET_NAME}::")
set(JMESPATH_REQUIRED_BOOST_VERSION 1.65)
set(JMESPATH_REQUIRED_JSON_VERSION 3.4.0)
# set compiler specific flags
if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"))
    set(JMESPATH_COMPILE_FLAGS "-Weverything -Wno-padded \
        -Wno-c++98-compat-pedantic -Wno-disabled-macro-expansion \
        -Wno-weak-vtables")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    #wd4348 - ingore warning C4348 (A template parameter was redefined.)
    #wd4068 - ingore warning C4068 (The compiler ignored an unrecognized pragma)
    #EHsc - set synchronous exception handling
    #bigobj - fix for error C1128 (increase the number of sections in .obj file)
    set(JMESPATH_COMPILE_FLAGS "/EHsc /wd4348 /wd4068 /bigobj")
endif()
enable_testing()

# find dependencies
find_package(Boost ${JMESPATH_REQUIRED_BOOST_VERSION} REQUIRED)
find_package(nlohmann_json ${JMESPATH_REQUIRED_JSON_VERSION} REQUIRED)

# add targets and variables in subdirectories
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(third_party)

# create the list of public header files
set(JMESPATH_PUBLIC_HEADER_FILES
    "include/jmespath/jmespath.h"
    "include/jmespath/expression.h"
    "include/jmespath/types.h"
    "include/jmespath/exceptions.h"
)

# set the include directories
list(APPEND JMESPATH_INCLUDE_DIRECTORIES
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

##
## JMESPATH LIBRARY TARGET
##
add_library(${JMESPATH_TARGET_NAME} ${JMESPATH_PUBLIC_HEADER_FILES}
    ${JMESPATH_SOURCE_FILES})
add_library("${JMESPATH_TARGET_NAMESPACE_NAME}${JMESPATH_TARGET_NAME}" ALIAS
    ${JMESPATH_TARGET_NAME})
target_include_directories(${JMESPATH_TARGET_NAME}
    PUBLIC ${JMESPATH_INCLUDE_DIRECTORIES})
set_target_properties(${JMESPATH_TARGET_NAME} PROPERTIES
    PUBLIC_HEADER "${JMESPATH_PUBLIC_HEADER_FILES}"
    COMPILE_FLAGS "${JMESPATH_COMPILE_FLAGS}"
    DEBUG_POSTFIX "d")
target_link_libraries(${JMESPATH_TARGET_NAME}
    PUBLIC Boost::boost nlohmann_json::nlohmann_json)
target_compile_definitions(${JMESPATH_TARGET_NAME}
    PUBLIC "BOOST_SPIRIT_UNICODE=1")
target_compile_features(${JMESPATH_TARGET_NAME} PUBLIC cxx_std_14)
if (${JMESPATH_COVERAGE_INFO})
    set_target_properties(${JMESPATH_TARGET_NAME} PROPERTIES
        COMPILE_FLAGS "-fprofile-arcs  -ftest-coverage"
        LINK_FLAGS "-fprofile-arcs  -ftest-coverage -lgcov")
endif ()

##
## INSTALL TARGET
##
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
# install target configuration
SET(JMESPATH_PACKAGE_NAME ${JMESPATH_TARGET_NAME})
SET(JMESPATH_CONFIG_INSTALL_DIR
    "lib/cmake/${JMESPATH_PACKAGE_NAME}")
SET(JMESPATH_CONFIG_FILE_NAME
    "${JMESPATH_PACKAGE_NAME}Config.cmake")
SET(JMESPATH_CONFIGVERSION_FILE_NAME
    "${JMESPATH_PACKAGE_NAME}ConfigVersion.cmake")
SET(JMESPATH_TARGETS_FILE_NAME "${JMESPATH_PACKAGE_NAME}Targets.cmake")

# install the jmespath library target
install(TARGETS ${JMESPATH_TARGET_NAME} EXPORT ${JMESPATH_PACKAGE_NAME}
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/${JMESPATH_PACKAGE_NAME}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/${JMESPATH_PACKAGE_NAME}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/${JMESPATH_PACKAGE_NAME}"
  INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${JMESPATH_PACKAGE_NAME}"
  PUBLIC_HEADER DESTINATION
  "${CMAKE_INSTALL_INCLUDEDIR}/${JMESPATH_PACKAGE_NAME}"
)
# generage package version file
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${JMESPATH_CONFIGVERSION_FILE_NAME}"
  COMPATIBILITY SameMajorVersion
)
# generate packe config file
configure_file("cmake/config.cmake.in"
  "${CMAKE_BINARY_DIR}/${JMESPATH_CONFIG_FILE_NAME}"
  @ONLY
)
# generate and install package targets file
install(EXPORT ${JMESPATH_PACKAGE_NAME}
  FILE ${JMESPATH_TARGETS_FILE_NAME}
  NAMESPACE ${JMESPATH_TARGET_NAMESPACE_NAME}
  DESTINATION ${JMESPATH_CONFIG_INSTALL_DIR}
)
# install package config and version files
install(FILES
    "${CMAKE_BINARY_DIR}/${JMESPATH_CONFIG_FILE_NAME}"
    "${CMAKE_BINARY_DIR}/${JMESPATH_CONFIGVERSION_FILE_NAME}"
    DESTINATION ${JMESPATH_CONFIG_INSTALL_DIR}
)
