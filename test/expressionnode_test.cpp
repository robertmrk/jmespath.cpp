/****************************************************************************
**
** Author: Róbert Márki <gsmiko@gmail.com>
** Copyright (c) 2016 Róbert Márki
**
** This file is part of the jmespath.cpp project which is distributed under
** the MIT License (MIT).
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to
** deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
** sell copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
** DEALINGS IN THE SOFTWARE.
**
****************************************************************************/
#include "fakeit.hpp"
#include "jmespath/ast/allnodes.h"
#include "jmespath/interpreter/abstractvisitor.h"

TEST_CASE("ExpressionNode")
{
    using namespace jmespath::ast;
    using namespace jmespath::interpreter;
    using namespace fakeit;

    SECTION("can be constructed")
    {
        SECTION("without parameters")
        {
            REQUIRE_NOTHROW(ExpressionNode{});
        }

        SECTION("with identifier")
        {
            IdentifierNode identifier;

            ExpressionNode expression{identifier};

            REQUIRE(expression == identifier);
        }

        SECTION("with raw string")
        {
            RawStringNode rawString;

            ExpressionNode expression{rawString};

            REQUIRE(expression == rawString);
        }

        SECTION("with literal")
        {
            LiteralNode literal;

            ExpressionNode expression{literal};

            REQUIRE(expression == literal);
        }

        SECTION("with subexpression")
        {
            SubexpressionNode subexpression;

            ExpressionNode expression{subexpression};

            REQUIRE(expression == subexpression);
        }

        SECTION("with index expression")
        {
            IndexExpressionNode indexExpression;

            ExpressionNode expression{indexExpression};

            REQUIRE(expression == indexExpression);
        }

        SECTION("with hash wildcard")
        {
            HashWildcardNode hashWildcard;

            ExpressionNode expression{hashWildcard};

            REQUIRE(expression == hashWildcard);
        }

        SECTION("with multiselect list")
        {
            MultiselectListNode multiselectList;

            ExpressionNode expression{multiselectList};

            REQUIRE(expression == multiselectList);
        }

        SECTION("with multiselect hash")
        {
            MultiselectHashNode multiselectHash;

            ExpressionNode expression{multiselectHash};

            REQUIRE(expression == multiselectHash);
        }
    }

    SECTION("accepts assignment of another ExpressionNode")
    {
        ExpressionNode node1;
        ExpressionNode node2{IdentifierNode{}};

        node1 = node2;

        REQUIRE(node1 == node2);
    }

    SECTION("accepts assignment of an ExpressionNode::Expression")
    {
        ExpressionNode node1;
        IdentifierNode node2;

        node1 = node2;

        REQUIRE(node1 == node2);
    }

    SECTION("accepts visitor")
    {
        ExpressionNode node{IdentifierNode{}};
        Mock<AbstractVisitor> visitor;
        When(OverloadedMethod(visitor, visit, void(IdentifierNode*)))
                .AlwaysReturn();

        node.accept(&visitor.get());

        Verify(OverloadedMethod(visitor, visit, void(IdentifierNode*))).Once();
    }
}
