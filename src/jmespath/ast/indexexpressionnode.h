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
#ifndef INDEXEXPRESSIONNODE_H
#define INDEXEXPRESSIONNODE_H
#include "jmespath/ast/binarynode.h"
#include "jmespath/ast/variantnode.h"
#include "jmespath/ast/expressionnode.h"
#include <boost/fusion/include/adapt_struct.hpp>

namespace jmespath { namespace ast {

class ArrayItemNode;
/**
 * @brief The IndexExpressionNode class represents a JMESPath index expression.
 */
class IndexExpressionNode : public BinaryNode<ExpressionNode,
        VariantNode<boost::recursive_wrapper<ArrayItemNode> > >
{
public:
    /**
     * @brief Construct an empty IndexExpressionNode object.
     */
    IndexExpressionNode();
    /**
     * @brief Constructs an IndexExpressionNode object with the given
     * @a subexpression as its right hand child.
     * @param subexpression The right hand child of the node.
     */
    IndexExpressionNode(const RightHandType& subexpression);
    /**
     * @brief Constructs an IndexExpressionNode object with the given
     * @a expression as it left hand child and @a subexpression as its right
     * hand child.
     * @param expression The left hand child node.
     * @param subexpression The right hand child node.
     */
    IndexExpressionNode(const LeftHandType& expression,
                        const RightHandType& subexpression);
};
}} // namespace jmespath::ast

BOOST_FUSION_ADAPT_STRUCT(
    jmespath::ast::IndexExpressionNode,
    (jmespath::ast::IndexExpressionNode::LeftHandType, leftExpression)
    (jmespath::ast::IndexExpressionNode::RightHandType, rightExpression)
)
#endif // INDEXEXPRESSIONNODE_H
