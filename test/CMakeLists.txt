cmake_minimum_required(VERSION 3.0)

set(JMESPATH_UNITTEST_TARGET_NAME unit)
set(JMESPATH_COMPLIANCETEST_TARGET_NAME compliance)
set(JMESPATH_COPY_COMPLIANCETEST_FILES_TARGET_NAME copy_tests)

add_executable(${JMESPATH_UNITTEST_TARGET_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/unit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/expression_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/grammar_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/search_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/expressionnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/identifiernode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawstringnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/variantnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interpreter_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interpreter_test2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/variantvisitoradaptor_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/subexpressionnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/literalnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/indexexpressionnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/arrayitemnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flattenoperatornode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/noderank_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/binaryexpressionnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bracketspecifiernode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/insertnodeaction_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sliceexpressionnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/listwildcardnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hashwildcardnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/multiselectlistnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/multiselecthashnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/notexpressionnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/comparatorexpressionnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/orexpressionnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/andexpressionnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parenexpressionnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pipeexpressionnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/currentnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filterexpressionnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/functionexpressionnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/expressionargumentnode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/leftchildextractor_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/nodeinsertpolicy_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/nodeinsertcondition_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/leftedgeiterator_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/appendutf8action_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/appendescapesequenceaction_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/encodesurrogatepairaction_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/contextvaluevisitoradaptor_test.cpp)
target_link_libraries(${JMESPATH_UNITTEST_TARGET_NAME}
    ${JMESPATH_TARGET_NAME} Catch2)
target_link_libraries(${JMESPATH_UNITTEST_TARGET_NAME}
    ${JMESPATH_TARGET_NAME} FakeIt)
set_target_properties(${JMESPATH_UNITTEST_TARGET_NAME} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON)
if (${JMESPATH_COVERAGE_INFO})
    set_target_properties(${JMESPATH_UNITTEST_TARGET_NAME} PROPERTIES
        COMPILE_FLAGS "-fprofile-arcs  -ftest-coverage"
        LINK_FLAGS "-fprofile-arcs  -ftest-coverage -lgcov")
endif ()
add_test(NAME ${JMESPATH_UNITTEST_TARGET_NAME}
    COMMAND ${JMESPATH_UNITTEST_TARGET_NAME})

add_executable(${JMESPATH_COMPLIANCETEST_TARGET_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/compliance.cpp)
set_target_properties(${JMESPATH_COMPLIANCETEST_TARGET_NAME} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON)
target_link_libraries(${JMESPATH_COMPLIANCETEST_TARGET_NAME}
    ${JMESPATH_TARGET_NAME} Catch2)
target_link_libraries(${JMESPATH_COMPLIANCETEST_TARGET_NAME}
    ${JMESPATH_TARGET_NAME} FakeIt)
add_test(NAME ${JMESPATH_COMPLIANCETEST_TARGET_NAME}
    COMMAND ${JMESPATH_COMPLIANCETEST_TARGET_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

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
