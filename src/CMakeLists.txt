cmake_minimum_required(VERSION 3.0)

set(JMESPATH_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/jmespath)
set(JMESPATH_DETAIL_SOURCE_DIR ${JMESPATH_SOURCE_DIR}/detail)
set(JMESPATH_PARSER_SOURCE_DIR ${JMESPATH_SOURCE_DIR}/parser)
set(JMESPATH_AST_SOURCE_DIR ${JMESPATH_SOURCE_DIR}/ast)
set(JMESPATH_INTERPRETER_SOURCE_DIR ${JMESPATH_SOURCE_DIR}/interpreter)

list(APPEND JMESPATH_SOURCE_FILES
    ${JMESPATH_SOURCE_DIR}/jmespath.cpp
    ${JMESPATH_SOURCE_DIR}/expression.h
    ${JMESPATH_SOURCE_DIR}/expression.cpp
    ${JMESPATH_DETAIL_SOURCE_DIR}/types.h
    ${JMESPATH_DETAIL_SOURCE_DIR}/exceptions.h
    ${JMESPATH_PARSER_SOURCE_DIR}/grammar.h
    ${JMESPATH_PARSER_SOURCE_DIR}/parser.h
    ${JMESPATH_PARSER_SOURCE_DIR}/noderank.h
    ${JMESPATH_PARSER_SOURCE_DIR}/insertnodeaction.h
    ${JMESPATH_PARSER_SOURCE_DIR}/nodechildextractionpolicy.h
    ${JMESPATH_PARSER_SOURCE_DIR}/nodeinsertpolicy.h
    ${JMESPATH_PARSER_SOURCE_DIR}/nodeinsertcondition.h
    ${JMESPATH_AST_SOURCE_DIR}/allnodes.h
    ${JMESPATH_AST_SOURCE_DIR}/abstractnode.h
    ${JMESPATH_AST_SOURCE_DIR}/binaryexpressionnode.h
    ${JMESPATH_AST_SOURCE_DIR}/binaryexpressionnode.cpp
    ${JMESPATH_AST_SOURCE_DIR}/variantnode.h
    ${JMESPATH_AST_SOURCE_DIR}/expressionnode.h
    ${JMESPATH_AST_SOURCE_DIR}/expressionnode.cpp
    ${JMESPATH_AST_SOURCE_DIR}/identifiernode.h
    ${JMESPATH_AST_SOURCE_DIR}/identifiernode.cpp
    ${JMESPATH_AST_SOURCE_DIR}/rawstringnode.h
    ${JMESPATH_AST_SOURCE_DIR}/rawstringnode.cpp
    ${JMESPATH_AST_SOURCE_DIR}/variantvisitoradaptor.h
    ${JMESPATH_AST_SOURCE_DIR}/variantvisitoradaptor.cpp
    ${JMESPATH_AST_SOURCE_DIR}/literalnode.h
    ${JMESPATH_AST_SOURCE_DIR}/literalnode.cpp
    ${JMESPATH_AST_SOURCE_DIR}/subexpressionnode.h
    ${JMESPATH_AST_SOURCE_DIR}/subexpressionnode.cpp
    ${JMESPATH_AST_SOURCE_DIR}/indexexpressionnode.h
    ${JMESPATH_AST_SOURCE_DIR}/indexexpressionnode.cpp
    ${JMESPATH_AST_SOURCE_DIR}/arrayitemnode.h
    ${JMESPATH_AST_SOURCE_DIR}/arrayitemnode.cpp
    ${JMESPATH_AST_SOURCE_DIR}/flattenoperatornode.h
    ${JMESPATH_AST_SOURCE_DIR}/flattenoperatornode.cpp
    ${JMESPATH_AST_SOURCE_DIR}/bracketspecifiernode.h
    ${JMESPATH_AST_SOURCE_DIR}/bracketspecifiernode.cpp
    ${JMESPATH_AST_SOURCE_DIR}/sliceexpressionnode.h
    ${JMESPATH_AST_SOURCE_DIR}/sliceexpressionnode.cpp
    ${JMESPATH_AST_SOURCE_DIR}/listwildcardnode.h
    ${JMESPATH_AST_SOURCE_DIR}/listwildcardnode.cpp
    ${JMESPATH_AST_SOURCE_DIR}/hashwildcardnode.h
    ${JMESPATH_AST_SOURCE_DIR}/hashwildcardnode.cpp
    ${JMESPATH_AST_SOURCE_DIR}/multiselectlistnode.h
    ${JMESPATH_AST_SOURCE_DIR}/multiselectlistnode.cpp
    ${JMESPATH_AST_SOURCE_DIR}/multiselecthashnode.h
    ${JMESPATH_AST_SOURCE_DIR}/multiselecthashnode.cpp
    ${JMESPATH_AST_SOURCE_DIR}/notexpressionnode.h
    ${JMESPATH_AST_SOURCE_DIR}/notexpressionnode.cpp
    ${JMESPATH_AST_SOURCE_DIR}/comparatorexpressionnode.h
    ${JMESPATH_AST_SOURCE_DIR}/comparatorexpressionnode.cpp
    ${JMESPATH_AST_SOURCE_DIR}/orexpressionnode.h
    ${JMESPATH_AST_SOURCE_DIR}/orexpressionnode.cpp
    ${JMESPATH_AST_SOURCE_DIR}/andexpressionnode.h
    ${JMESPATH_AST_SOURCE_DIR}/andexpressionnode.cpp
    ${JMESPATH_AST_SOURCE_DIR}/parenexpressionnode.h
    ${JMESPATH_AST_SOURCE_DIR}/parenexpressionnode.cpp
    ${JMESPATH_AST_SOURCE_DIR}/pipeexpressionnode.h
    ${JMESPATH_AST_SOURCE_DIR}/pipeexpressionnode.cpp
    ${JMESPATH_AST_SOURCE_DIR}/currentnode.h
    ${JMESPATH_AST_SOURCE_DIR}/currentnode.cpp
    ${JMESPATH_AST_SOURCE_DIR}/filterexpressionnode.h
    ${JMESPATH_AST_SOURCE_DIR}/filterexpressionnode.cpp
    ${JMESPATH_AST_SOURCE_DIR}/functionexpressionnode.h
    ${JMESPATH_AST_SOURCE_DIR}/functionexpressionnode.cpp
    ${JMESPATH_AST_SOURCE_DIR}/expressionargumentnode.h
    ${JMESPATH_AST_SOURCE_DIR}/expressionargumentnode.cpp
    ${JMESPATH_INTERPRETER_SOURCE_DIR}/abstractvisitor.h
    ${JMESPATH_INTERPRETER_SOURCE_DIR}/interpreter.h
    ${JMESPATH_INTERPRETER_SOURCE_DIR}/interpreter.cpp)
set(JMESPATH_SOURCE_FILES ${JMESPATH_SOURCE_FILES} PARENT_SCOPE)

list(APPEND JMESPATH_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR})
set(JMESPATH_INCLUDE_DIRECTORIES ${JMESPATH_INCLUDE_DIRECTORIES} PARENT_SCOPE)
