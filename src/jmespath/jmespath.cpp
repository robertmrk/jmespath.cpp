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
#include "jmespath/jmespath.h"
#include "jmespath/parser/parser.h"
#include "jmespath/parser/grammar.h"
#include "jmespath/interpreter/interpreter.h"

namespace jmespath {

Json search(const String &searchExpression, const Json &document)
{
    using parser::Parser;
    using parser::Grammar;
    using interpreter::Interpreter;

    // return null on empty searchExpression
    if (searchExpression.empty())
    {
        return {};
    }

    // create a parser
    Parser<Grammar> parser;
    // parse the searchExpression and create an AST
    auto astRoot = parser.parse(searchExpression);
    // create an interpreter with the given JSON document
    Interpreter interpreter{document};
    // evaluate the expression by calling visit with the root of the AST
    interpreter.visit(&astRoot);

    // return the current evaluation context as the result
    return interpreter.currentContext();
}
} // namespace jmespath
