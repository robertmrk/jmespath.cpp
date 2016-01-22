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
#ifndef SUBEXPRESSIONNODE_H
#define SUBEXPRESSIONNODE_H
#include "jmespath/ast/node.h"
#include "jmespath/ast/expressionnode.h"

namespace jmespath { namespace ast {

/**
 * @brief The SubexpressionNode class represents a JMESPath subexpression.
 */
class SubexpressionNode : public Node
{
public:
    using Subexpression
        = VariantNode<boost::recursive_wrapper<IdentifierNode> >;
    /**
     * @brief Constructs and empty SubexpressionNode object.
     */
    SubexpressionNode();
    /**
     * @brief Constructs an SubexpressionNode object with the given left hand
     * side @a expression and a right hand side @a subexpression.
     * @param expression The node's left hand side child expression
     * @param subexpression The node's right hand side child expression
     */
    SubexpressionNode(const ExpressionNode& expression,
                      const Subexpression& subexpression = boost::blank{});
    /**
     * @brief Equality compares this node to the \a other
     * @param other The node that should be compared.
     * @return Returns true if this object is equal to the \a other, otherwise
     * false
     */
    bool operator==(const SubexpressionNode& other) const;
    void accept(interpreter::AbstractVisitor* visitor) override;
    /**
     * @brief The node's left hand side child expression
     */
    ExpressionNode expression;
    /**
     * @brief The node's right hand side child expression
     */
    Subexpression subexpression;
};
}} // namespace jmespath::ast
#endif // SUBEXPRESSIONNODE_H
