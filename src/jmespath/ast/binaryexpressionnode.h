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
#ifndef BINARYEXPRESSIONNODE_H
#define BINARYEXPRESSIONNODE_H
#include "jmespath/ast/abstractnode.h"
#include "jmespath/ast/expressionnode.h"

namespace jmespath { namespace ast {

/**
 * @brief The BinaryExpressionNode class is the base class for all node types
 * which consist of a left and a right hand side expression.
 * @tparam ExpressionNode The left hand node's type.
 * @tparam ExpressionNode The right hand node's type.
 */
class BinaryExpressionNode : public AbstractNode
{
public:
    /**
     * @brief Constructs an empty BinaryExpressionNode object.
     */
    BinaryExpressionNode();
    /**
     * @brief Constructs a BinaryExpressionNode object with the given @a leftExpressin
     * and @a rightExpression as its children.
     * @param leftExpression Left hand expression of the node.
     * @param rightExpression Right hand expression of the node.
     */
    BinaryExpressionNode(const ExpressionNode& leftExpression,
               const ExpressionNode& rightExpression);
    /**
     * @brief Equality compares this node to the \a other
     * @param other The node that should be compared.
     * @return Returns true if this object is equal to the \a other, otherwise
     * false
     */
    bool operator==(const BinaryExpressionNode& other) const;
    /**
     * @brief Calls the accept method with the given @a visitor as the parameter
     * on the node's child expressions.
     * @param visitor A visitor implementation.
     */
    void accept(interpreter::AbstractVisitor *visitor) override;
    /**
     * @brief Reports wheather the right hand side expression is projected onto
     * the result of the operation or not.
     * @return Returns true if the right hand expression is projected, otherwise
     * returns false.
     */
    virtual bool isProjection() const = 0;
    /**
     * @brief The left hand expression of the node.
     */
    ExpressionNode leftExpression;
    /**
     * @brief The right hand expression of the node.
     */
    ExpressionNode rightExpression;
};
}} // namespace jmespath::ast
#endif // BINARYEXPRESSIONNODE_H
