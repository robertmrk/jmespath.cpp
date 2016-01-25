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
#ifndef ROTATENODELEFTACTION_H
#define ROTATENODELEFTACTION_H

namespace jmespath { namespace parser {

/**
 * @brief The RotateNodeLeftAction struct is a functor that rotates an AST node
 * left, it can be used to make a right leaning subtree left leaning.
 */
struct RotateNodeLeftAction
{
    /**
     * Executes the left rotation operation on @a node, by making @a node the
     * left child of @a rightChild and making @a rightGrandChild the right
     * child of @a rightChild node. The final value of @a rightChild will become
     * the only child of the @a node.
     */
    template <typename T1, typename T2, typename T3>
    void operator()(T1& node,
                    T2& rightChild,
                    const T3& rightGrancChild) const
    {
        rightChild.leftExpression = node;
        rightChild.rightExpression = rightGrancChild;
        node = T1{rightChild};
    }
};
}} // namespace jmespath::parser
#endif // ROTATENODELEFTACTION_H
