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
#ifndef BINARYNODE_H
#define BINARYNODE_H
#include "jmespath/ast/abstractnode.h"

namespace jmespath { namespace ast {

/**
 * @brief The BinaryNode class is the base class for all node types which
 * consist of a left and a right hand side expression.
 * @tparam T1 The left hand node's type.
 * @tparam T2 The right hand node's type.
 */
template <typename T1, typename T2>
class BinaryNode : public AbstractNode
{
public:
    using LeftHandType = T1;
    using RightHandType = T2;
    /**
     * @brief Constructs an empty BinaryNode object.
     */
    BinaryNode()
        : AbstractNode()
    {
    }
    /**
     * @brief Constructs a BinaryNode object with the given @a leftExpressin
     * and @a rightExpression as its children.
     * @param leftExpression Left hand expression of the node.
     * @param rightExpression Right hand expression of the node.
     */
    BinaryNode(const T1& leftExpression, const T2& rightExpression)
        : AbstractNode(),
          leftExpression(leftExpression),
          rightExpression(rightExpression)
    {
    }
    /**
     * @brief Equality compares this node to the \a other
     * @param other The node that should be compared.
     * @return Returns true if this object is equal to the \a other, otherwise
     * false
     */
    bool operator==(const BinaryNode<T1, T2>& other) const
    {
        if (this != &other)
        {
            return (leftExpression == other.leftExpression)
                    && (rightExpression == other.rightExpression);
        }
        return true;
    }
    /**
     * @brief Calls the accept method with the given @a visitor as the parameter
     * on the node's child expressions.
     * @param visitor A visitor implementation.
     */
    void accept(interpreter::AbstractVisitor *visitor) override
    {
        leftExpression.accept(visitor);
        rightExpression.accept(visitor);
    }
    /**
     * @brief The left hand expression of the node.
     */
    T1 leftExpression;
    /**
     * @brief The right hand expression of the node.
     */
    T2 rightExpression;
};
}} // namespace jmespath::ast
#endif // BINARYNODE_H
