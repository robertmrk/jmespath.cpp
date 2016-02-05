cmake_minimum_required(VERSION 3.0)

set(JMESPATH_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/jmespath)
set(JMESPATH_DETAIL_SOURCE_DIR ${JMESPATH_SOURCE_DIR}/detail)
set(JMESPATH_PARSER_SOURCE_DIR ${JMESPATH_SOURCE_DIR}/parser)
set(JMESPATH_AST_SOURCE_DIR ${JMESPATH_SOURCE_DIR}/ast)
set(JMESPATH_INTERPRETER_SOURCE_DIR ${JMESPATH_SOURCE_DIR}/interpreter)

list(APPEND JMESPATH_SOURCE_FILES
    ${JMESPATH_SOURCE_DIR}/jmespath.cpp
    ${JMESPATH_DETAIL_SOURCE_DIR}/types.h
    ${JMESPATH_DETAIL_SOURCE_DIR}/exceptions.h
    ${JMESPATH_PARSER_SOURCE_DIR}/grammar.h
    ${JMESPATH_PARSER_SOURCE_DIR}/parser.h
    ${JMESPATH_PARSER_SOURCE_DIR}/noderank.h
    ${JMESPATH_PARSER_SOURCE_DIR}/insertbinaryexpressionnodeaction.h
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
    ${JMESPATH_INTERPRETER_SOURCE_DIR}/abstractvisitor.h
    ${JMESPATH_INTERPRETER_SOURCE_DIR}/expressionevaluator.h
    ${JMESPATH_INTERPRETER_SOURCE_DIR}/expressionevaluator.cpp)
set(JMESPATH_SOURCE_FILES ${JMESPATH_SOURCE_FILES} PARENT_SCOPE)

list(APPEND JMESPATH_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR})
set(JMESPATH_INCLUDE_DIRECTORIES ${JMESPATH_INCLUDE_DIRECTORIES} PARENT_SCOPE)
