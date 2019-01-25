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
#include "jmespath/interpreter/contextvaluevisitoradaptor.h"
#include "jmespath/interpreter/interpreter2.h"
#include "jmespath/exceptions.h"
#include "jmespath/ast/identifiernode.h"


class Interpreter2Stub : public jmespath::interpreter::Interpreter2
{
public:
    virtual void visit(const jmespath::ast::IdentifierNode *node,
                       const jmespath::Json& context) {}
    virtual void visit(const jmespath::ast::IdentifierNode *node,
                       jmespath::Json&& context) {}
};

TEST_CASE("ContextValueVisitorAdaptor")
{
    using namespace jmespath;
    using namespace jmespath::ast;
    using namespace jmespath::interpreter;
    using namespace fakeit;
    using AdaptorType = ContextValueVisitorAdaptor<IdentifierNode,
                                                   Interpreter2Stub>;

    SECTION("can be constructed with visito and node")
    {
        Interpreter2Stub visitor;
        IdentifierNode node {"identifier"};

        REQUIRE_NOTHROW(AdaptorType(&visitor, &node));
    }

    SECTION("can't be constructed with nullptr visitor")
    {
        IdentifierNode node {"identifier"};

        REQUIRE_THROWS_AS(AdaptorType(nullptr, &node), InvalidAgrument);
    }

    SECTION("can't be constructed with nullptr node")
    {
        Interpreter2Stub visitor;

        REQUIRE_THROWS_AS(AdaptorType(&visitor, nullptr), InvalidAgrument);
    }

    SECTION("calls visit method of visitor with node and json lvalue ref")
    {
        Json value{"value"};
        ContextValue contextValue{std::cref(value)};
        IdentifierNode node{"identifier"};
        Mock<Interpreter2Stub> visitor;
        When(OverloadedMethod(visitor, visit, void(const IdentifierNode*,
                                                   const Json&)))
                .AlwaysReturn();

        boost::apply_visitor(AdaptorType(&visitor.get(), &node), contextValue);

        Verify(OverloadedMethod(visitor, visit,
                                void(const IdentifierNode*, const Json&))
               .Using(&node, boost::get<JsonRef>(contextValue).get()))
                .Once();
        VerifyNoOtherInvocations(visitor);
    }

    SECTION("calls visit method of visitor with node and json rvalue ref")
    {
        ContextValue contextValue{Json{"value"}};
        IdentifierNode node{"identifier"};
        Mock<Interpreter2Stub> visitor;
        When(OverloadedMethod(visitor, visit, void(const IdentifierNode*,
                                                   Json&&)))
                .AlwaysReturn();

        boost::apply_visitor(AdaptorType(&visitor.get(), &node), contextValue);

        Verify(OverloadedMethod(visitor, visit,
                                void(const IdentifierNode*, Json&&))
               .Using(&node, std::move(boost::get<Json&>(contextValue))))
                .Once();
        VerifyNoOtherInvocations(visitor);
    }
}
