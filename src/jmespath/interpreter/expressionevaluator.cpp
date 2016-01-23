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
#include "jmespath/interpreter/expressionevaluator.h"
#include "jmespath/ast/identifiernode.h"
#include "jmespath/ast/rawstringnode.h"
#include "jmespath/ast/expressionnode.h"
#include "jmespath/ast/literalnode.h"
#include "jmespath/ast/subexpressionnode.h"

namespace jmespath { namespace interpreter {

ExpressionEvaluator::ExpressionEvaluator()
    : AbstractVisitor()
{
}

ExpressionEvaluator::ExpressionEvaluator(const Json &contextValue)
    : AbstractVisitor()
{
    setContext(contextValue);
}

void ExpressionEvaluator::setContext(const Json &value)
{
    m_context = value;
}

Json ExpressionEvaluator::currentContext() const
{
    return m_context;
}

void ExpressionEvaluator::visit(ast::AbstractNode *node)
{
    node->accept(this);
}

void ExpressionEvaluator::visit(ast::Node *node)
{
    node->accept(this);
}

void ExpressionEvaluator::visit(ast::ExpressionNode *node)
{
    node->accept(this);
}

void ExpressionEvaluator::visit(ast::IdentifierNode *node)
{
    m_context = m_context[node->identifier];
}

void ExpressionEvaluator::visit(ast::RawStringNode *node)
{
    m_context = node->rawString;
}

void ExpressionEvaluator::visit(ast::LiteralNode *node)
{
    m_context = Json::parse(node->literal);
}

void ExpressionEvaluator::visit(ast::SubexpressionNode *node)
{
    node->accept(this);
}
}} // namespace jmespath::interpreter
