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
#ifndef EXPRESSIONNODE_H
#define EXPRESSIONNODE_H
#include "jmespath/ast/node.h"
#include "jmespath/ast/variantnode.h"
#include <boost/variant.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

namespace jmespath { namespace ast {

class IdentifierNode;
class RawStringNode;
class LiteralNode;
class SubexpressionNode;
class IndexExpressionNode;
/**
 * @brief The ExpressionNode class represents a JMESPath expression.
 */
class ExpressionNode : public Node
{
public:
    using Expression = VariantNode<boost::recursive_wrapper<IdentifierNode>,
        boost::recursive_wrapper<RawStringNode>,
        boost::recursive_wrapper<LiteralNode>,
        boost::recursive_wrapper<SubexpressionNode>,
        boost::recursive_wrapper<IndexExpressionNode> >;

    /**
     * @brief Constructs an empy ExpressionNode object
     */
    ExpressionNode();
    /**
     * @brief Constructs an ExpressionNode object with its child expression
     * initialized to \a expression
     * @param expression The node's child expression
     */
    ExpressionNode(const Expression& expression);
    /**
     * @brief Equality compares this node to the \a other
     * @param other The node that should be compared.
     * @return Returns true if this object is equal to the \a other, otherwise
     * false
     */
    bool operator==(const ExpressionNode& other) const;
    void accept(interpreter::AbstractVisitor* visitor) override;
    /**
     * @brief The node's child expression
     */
    Expression expression;
};
}} // namespace jmespath::ast

BOOST_FUSION_ADAPT_STRUCT(
    jmespath::ast::ExpressionNode,
    (jmespath::ast::ExpressionNode::Expression, expression)
)
#endif // EXPRESSIONNODE_H
