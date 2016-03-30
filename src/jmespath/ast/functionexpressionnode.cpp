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
#include "jmespath/ast/functionexpressionnode.h"
#include "jmespath/ast/allnodes.h"
#include "jmespath/interpreter/abstractvisitor.h"

namespace jmespath { namespace ast {

FunctionExpressionNode::FunctionExpressionNode()
    : AbstractNode()
{
}

FunctionExpressionNode::FunctionExpressionNode(const String &name,
        const std::initializer_list<ArgumentType> &argumentList)
    : AbstractNode(),
      functionName(name),
      arguments(argumentList)
{
}

void FunctionExpressionNode::accept(interpreter::AbstractVisitor *visitor) const
{
    visitor->visit(this);
}

bool FunctionExpressionNode::operator==(
        const FunctionExpressionNode &other) const
{
    if (this != &other)
    {
        return (functionName == other.functionName)
                && (arguments == other.arguments);
    }
    return true;
}
}} // namespace jmespath::ast
