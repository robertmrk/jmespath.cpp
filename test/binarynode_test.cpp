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
#include "jmespath/ast/binarynode.h"
#include "jmespath/ast/identifiernode.h"
#include "jmespath/interpreter/abstractvisitor.h"
#include "jmespath/interpreter/expressionevaluator.h"

TEST_CASE("BinaryNode")
{
    using namespace jmespath::ast;
    using namespace fakeit;
    using jmespath::interpreter::AbstractVisitor;
    using BinaryNodeType = BinaryNode<IdentifierNode, IdentifierNode>;

    SECTION("can be default constructed")
    {
        REQUIRE_NOTHROW(BinaryNodeType{});
    }

    SECTION("can be constructed with left and right expression")
    {
        IdentifierNode id1{"id1"};
        IdentifierNode id2{"id2"};

        BinaryNodeType node{id1, id2};

        REQUIRE(node.leftExpression == id1);
        REQUIRE(node.rightExpression == id2);
    }

    SECTION("can be compared for equality")
    {
        BinaryNodeType node1{IdentifierNode{"id1"},
                             IdentifierNode{"id2"}};
        BinaryNodeType node2;
        node2 = node1;

        REQUIRE(node1 == node2);
        REQUIRE(node1 == node1);
    }

    SECTION("accepts visitor")
    {
        IdentifierNode id1;
        IdentifierNode id2;
        BinaryNodeType node{id1, id2};
        Mock<AbstractVisitor> visitor;
        When(OverloadedMethod(visitor, visit, void(IdentifierNode*)))
                .AlwaysReturn();

        node.accept(&visitor.get());

        Verify(OverloadedMethod(visitor, visit, void(IdentifierNode*)))
                .Exactly(2);
    }
}
