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
#include "jmespath/ast/subexpressionnode.h"
#include "jmespath/ast/identifiernode.h"
#include "jmespath/ast/rawstringnode.h"
#include "jmespath/ast/literalnode.h"
#include "jmespath/ast/expressionnode.h"

TEST_CASE("SubexpressionNode")
{
    using namespace jmespath::ast;
    using namespace jmespath::interpreter;
    using namespace fakeit;

    SECTION("can be default constructed")
    {
        REQUIRE_NOTHROW(SubexpressionNode{});
    }

    SECTION("can be constructed with expression")
    {
        ExpressionNode expression{};
        SubexpressionNode node{expression};

        REQUIRE(node.expression == expression);
    }

    SECTION("can be constructed with expression and identifier")
    {
        ExpressionNode expression{};
        IdentifierNode identifier{};
        SubexpressionNode node{expression, identifier};

        REQUIRE(node.expression == expression);
        REQUIRE(node.subexpression == identifier);
    }

    SECTION("can be compared for equality")
    {
        ExpressionNode expression{};
        IdentifierNode identifier{"value"};
        expression.expression = identifier;
        SubexpressionNode node1{expression, identifier};
        SubexpressionNode node2{expression, identifier};

        REQUIRE(node1 == node2);
        REQUIRE(node1 == node1);
    }

    SECTION("accepts visitor")
    {
        ExpressionNode expression{IdentifierNode{}};
        IdentifierNode identifier;
        SubexpressionNode node{expression, identifier};
        Mock<AbstractVisitor> visitor;
        When(OverloadedMethod(visitor, visit, void(IdentifierNode*)))
                .AlwaysReturn();

        node.accept(&visitor.get());

        Verify(OverloadedMethod(visitor, visit, void(IdentifierNode*)))
                .Exactly(2);
    }
}
