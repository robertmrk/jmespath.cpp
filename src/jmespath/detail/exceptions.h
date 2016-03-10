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
#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H
#include <stdexcept>
#include <boost/exception/all.hpp>

namespace jmespath { namespace detail {
/**
 * @brief InfoSearchExpression contains the JMESPath expression being evaluated
 */
using InfoSearchExpression
    = boost::error_info<struct tag_search_expression, std::string>;
/**
 * @brief InfoSyntaxErrorLocation contains the location of the syntax error
 * in the JMESPath expression
 */
using InfoSyntaxErrorLocation
    = boost::error_info<struct tag_syntax_error_location, int>;
/**
 * @brief InfoFunctionName contains the name of the built in JMESpath function.
 */
using InfoFunctionName
    = boost::error_info<struct tag_function_name, std::string>;

/**
 * @brief The Exception struct is the common base class for
 * for all the exceptions thrown by the library
 */
struct Exception : virtual boost::exception, virtual std::exception {};
/**
 * @brief The SyntaxError struct represents a syntax error in
 * the evaluated expression
 */
struct SyntaxError : virtual Exception {};
/**
 * @brief The InvalidAgrument struct signals a function call with illegal
 * arguments
 */
struct InvalidAgrument : virtual Exception {};
/**
 * @brief The InvalidValue struct represents an invalid value in the JMESPath
 * expression.
 */
struct InvalidValue : virtual Exception {};
/**
 * @brief The UnknownFunction struct represents a call to a JMESPath built in
 * function which doesn't exists.
 */
struct UnknownFunction : virtual Exception {};
/**
 * @brief The InvalidFunctionArgumentArity struct signals the a JMESPath built
 * in function was called with an unexpected number of arguments.
 */
struct InvalidFunctionArgumentArity : virtual Exception {};
/**
 * @brief The InvalidFunctionArgumentType struct represents a call to a JMESPath
 * built in function with an unexpected type of argument.
 */
struct InvalidFunctionArgumentType : virtual Exception {};
}} // namespace jmespath::detail
#endif // EXCEPTIONS_H
