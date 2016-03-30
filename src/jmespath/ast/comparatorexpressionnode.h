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
#ifndef COMPARATOREXPRESSIONNODE_H
#define COMPARATOREXPRESSIONNODE_H
#include "jmespath/ast/binaryexpressionnode.h"
#include <boost/fusion/include/adapt_struct.hpp>

namespace jmespath { namespace ast {

/**
 * @brief The ComparatorExpressionNode class represents a JMESPath comparator
 * expression.
 */
class ComparatorExpressionNode : public BinaryExpressionNode
{
public:
    /**
     * @brief The Comparator enum defines the available comparison operators.
     */
    enum class Comparator
    {
        Unknown,
        Less,
        LessOrEqual,
        Equal,
        GreaterOrEqual,
        Greater,
        NotEqual
    };
    /**
     * @brief Constructs an empty ComparatorExpressionNode object.
     */
    ComparatorExpressionNode();
    /**
     * @brief Constructs a ComparatorExpressionNode object with the given @a
     * leftExpression, @a comparator and @a rightExpression.
     * @param left The node's left hand child expression.
     * @param valueComparator The type of comparison operator to use for
     * comparing the results of the left and right hand child expressions.
     * @param right The node's right hand child expression.
     */
    ComparatorExpressionNode(const ExpressionNode& left,
                             Comparator valueComparator,
                             const ExpressionNode& right);
    /**
     * @brief Equality compares this node to the \a other
     * @param other The node that should be compared.
     * @return Returns true if this object is equal to the \a other, otherwise
     * false
     */
    bool operator ==(const ComparatorExpressionNode& other) const;
    /**
     * @brief Returns whather this expression requires the projection of
     * subsequent expressions.
     * @return Returns true if projection is required, otherwise returns false.
     */
    bool isProjection() const override;
    /**
     * @brief Reports whether the node should stop an ongoing projection or
     * not.
     * @return Returns true if the node should stop an ongoing projection,
     * otherwise returns false.
     */
    bool stopsProjection() const override;
    /**
     * @brief Calls the visit method of the given \a visitor with the
     * dynamic type of the node.
     * @param visitor A visitor implementation
     */
    void accept(interpreter::AbstractVisitor* visitor) const override;
    /**
     * @brief The type of comparator associated with the expression.
     */
    Comparator comparator;
};
}} // namespace jmespath::ast

BOOST_FUSION_ADAPT_STRUCT(
    jmespath::ast::ComparatorExpressionNode,
    (jmespath::ast::ExpressionNode, leftExpression)
    (jmespath::ast::ComparatorExpressionNode::Comparator, comparator)
    (jmespath::ast::ExpressionNode, rightExpression)
)
#endif // COMPARATOREXPRESSIONNODE_H
