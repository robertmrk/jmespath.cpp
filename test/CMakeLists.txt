cmake_minimum_required(VERSION 3.0)

set(JMESPATH_UNITTEST_TARGET_NAME unit)
set(JMESPATH_COMPLIANCETEST_TARGET_NAME compliance)
set(JMESPATH_COPY_COMPLIANCETEST_FILES_TARGET_NAME copy_tests)
set(JMESPATH_COVERAGE_INFO OFF CACHE BOOL "Generate code coverage information")

add_executable(${JMESPATH_UNITTEST_TARGET_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/unit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/grammar_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/search_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/node_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/expressionnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/identifiernode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawstringnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/variantnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/expressionevaluator_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/variantvisitoradaptor_test.cpp)
target_link_libraries(${JMESPATH_UNITTEST_TARGET_NAME} ${JMESPATH_TARGET_NAME})
set_target_properties(${JMESPATH_UNITTEST_TARGET_NAME} PROPERTIES
    COMPILE_DEFINITIONS "BOOST_SPIRIT_UNICODE=1")
if (${JMESPATH_COVERAGE_INFO})
    set_target_properties(${JMESPATH_UNITTEST_TARGET_NAME} PROPERTIES
        COMPILE_FLAGS "--coverage"
        LINK_FLAGS "--coverage")
endif ()

add_executable(${JMESPATH_COMPLIANCETEST_TARGET_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/compliance.cpp)
target_link_libraries(${JMESPATH_COMPLIANCETEST_TARGET_NAME}
    ${JMESPATH_TARGET_NAME})

if (UNIX)
    set(JMESPATH_COPY_COMPLIANCETEST_FILES_OPERATION "create_symlink")
elseif (WIN32)
    set(JMESPATH_COPY_COMPLIANCETEST_FILES_OPERATION "copy_directory")
endif ()

add_custom_target(${JMESPATH_COPY_COMPLIANCETEST_FILES_TARGET_NAME}
    ${CMAKE_COMMAND} -E ${JMESPATH_COPY_COMPLIANCETEST_FILES_OPERATION}
    ${CMAKE_CURRENT_SOURCE_DIR}/jmespath.test/tests
    ${CMAKE_CURRENT_BINARY_DIR}/compliance_tests)
add_dependencies(${JMESPATH_COMPLIANCETEST_TARGET_NAME}
    ${JMESPATH_COPY_COMPLIANCETEST_FILES_TARGET_NAME})
