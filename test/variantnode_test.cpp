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
#include "jmespath/ast/variantnode.h"
#include "jmespath/ast/identifiernode.h"
#include "jmespath/ast/node.h"

TEST_CASE("VariantNode")
{
    using namespace jmespath::ast;
    using namespace fakeit;

    SECTION("is default constructible")
    {
        REQUIRE_NOTHROW(VariantNode<Node>{});
    }

    SECTION("can be constructed with acceptable variant value")
    {
        REQUIRE_NOTHROW(VariantNode<Node>(Node{}));
    }

    SECTION("accepts variant value assignments")
    {
        VariantNode<Node> variantNode;

        REQUIRE_NOTHROW(variantNode = Node{});
    }

    SECTION("can be copy constructed")
    {
        VariantNode<Node> node;

        REQUIRE_NOTHROW(VariantNode<Node>(node));
    }

    SECTION("is copy assignable")
    {
        VariantNode<IdentifierNode> node1(IdentifierNode{"name"});
        VariantNode<IdentifierNode> node2;

        node2 = node1;

        bool result = node1.variant == node2.variant;
        REQUIRE(result);
    }

    SECTION("is comparable")
    {
        VariantNode<IdentifierNode> node1(IdentifierNode{"name"});
        VariantNode<IdentifierNode> node2;
        node2 = node1;

        REQUIRE(node1 == node2);
    }

    SECTION("is null when default constructed")
    {
        VariantNode<Node> node;

        REQUIRE(node.isNull());
    }

    SECTION("calls visit method of visitor on accept")
    {
        using jmespath::interpreter::AbstractVisitor;
        VariantNode<Node> node = Node{};
        Mock<AbstractVisitor> visitor;
        When(OverloadedMethod(visitor, visit, void(Node*))).AlwaysReturn();

        node.accept(&visitor.get());

        Verify(OverloadedMethod(visitor, visit, void(Node*))).Once();
        VerifyNoOtherInvocations(visitor);
    }
}
