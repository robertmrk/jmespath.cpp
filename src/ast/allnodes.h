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
#ifndef ALLNODES_H
#define ALLNODES_H
#include "src/ast/abstractnode.h"
#include "src/ast/expressionnode.h"
#include "src/ast/identifiernode.h"
#include "src/ast/rawstringnode.h"
#include "src/ast/literalnode.h"
#include "src/ast/subexpressionnode.h"
#include "src/ast/indexexpressionnode.h"
#include "src/ast/arrayitemnode.h"
#include "src/ast/variantnode.h"
#include "src/ast/binaryexpressionnode.h"
#include "src/ast/flattenoperatornode.h"
#include "src/ast/bracketspecifiernode.h"
#include "src/ast/sliceexpressionnode.h"
#include "src/ast/listwildcardnode.h"
#include "src/ast/hashwildcardnode.h"
#include "src/ast/multiselectlistnode.h"
#include "src/ast/multiselecthashnode.h"
#include "src/ast/notexpressionnode.h"
#include "src/ast/comparatorexpressionnode.h"
#include "src/ast/orexpressionnode.h"
#include "src/ast/andexpressionnode.h"
#include "src/ast/parenexpressionnode.h"
#include "src/ast/pipeexpressionnode.h"
#include "src/ast/currentnode.h"
#include "src/ast/filterexpressionnode.h"
#include "src/ast/functionexpressionnode.h"
#include "src/ast/expressionargumentnode.h"
#endif // ALLNODES_H
