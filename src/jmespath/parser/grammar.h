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
#ifndef GRAMMAR_H
#define GRAMMAR_H
#include "jmespath/detail/types.h"
#include "jmespath/ast/allnodes.h"
#include "jmespath/parser/noderank.h"
#include "jmespath/parser/insertbinaryexpressionnodeaction.h"
#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix.hpp>

/**
 * @namespace jmespath::parser
 * @brief Classes required for parsing JMESPath expressions
 */
namespace jmespath { namespace parser {

namespace qi = boost::spirit::qi;
namespace encoding = qi::unicode;
namespace phx = boost::phoenix;
using namespace detail;

/**
 * @brief The Grammar class contains the PEG rule definition based
 * on the EBNF specifications of JMESPath.
 *
 * The actual grammar is slightly modified compared to the specifications to
 * eliminate left recursion.
 * @tparam Iterator String iterator type
 * @tparam Skipper Character skipper parser type
 * @sa http://jmespath.org/specification.html#grammar
 */
template <typename Iterator, typename Skipper = encoding::space_type>
class Grammar : public qi::grammar<Iterator,
                                   ast::ExpressionNode(),
                                   qi::locals<ast::ExpressionNode>,
                                   Skipper>
{
public:
    /**
     * @brief Constructs a Grammar object
     */
    Grammar()
        : Grammar::base_type(m_expressionRule),
          m_rootNode(nullptr)
    {
        using encoding::char_;
        using qi::int_;
        using qi::lit;
        using qi::lexeme;
        using qi::int_parser;
        using qi::_val;
        using qi::_1;
        using qi::_2;
        using qi::_pass;
        using qi::_r1;
        using qi::_r2;
        using qi::eps;
        using qi::_a;
        using phx::at_c;
        using phx::if_;
        using phx::ref;

        // Since boost spirit doesn't support left recursive grammars, some of
        // the rules are converted into tail expressions. When a tail expression
        // is parsed and the parser returns from a recursion the
        // InsertBinaryExpressionNodeAction functor is used to compensate for
        // tail expressions. It inserts the binary expression nodes into the
        // appropriate position in the AST which leads to an easy to interpret
        // AST.

        // lazy function for inserting binary nodes to the appropriate position
        phx::function<InsertBinaryExpressionNodeAction> insertNode;
        phx::function<ReplaceIfNull> replaceIfNull;

        // match a standalone index expression or hash wildcard expression or
        // identifier or not expression or multiselect list or multiselect hash
        // or literal or a raw string, optionally followed by a subexpression
        // an index expression a hash wildcard subexpression and a comparator
        // expression
        m_expressionRule = eps[
                            if_(!ref(m_rootNode))[
                                ref(m_rootNode) = &_val
                            ]]
                >> (m_indexExpressionRule[insertNode(*ref(m_rootNode), _1)]
                    | m_hashWildcardRule[insertNode(*ref(m_rootNode), _1)]
                    | (m_identifierRule
                      | m_notExpressionRule
                      | m_multiselectListRule
                      | m_multiselectHashRule
                      | m_literalRule
                      | m_rawStringRule)[at_c<0>(_val) = _1, _a = _val])
            >> -m_subexpressionRule[insertNode(*ref(m_rootNode), _1, _a)]
            >> -m_indexExpressionRule[insertNode(*ref(m_rootNode), _1, _a)]
            >> -m_hashWildcardSubexpressionRule[insertNode(*ref(m_rootNode),
                                                           _1, _a)]
            >> -m_comparatorExpressionRule[insertNode(*ref(m_rootNode), _1, _a)]
            >> -m_orExpressionRule[insertNode(*ref(m_rootNode), _1, _a)];
        // match an identifier or multiselect list or multiselect hash preceded
        // by a dot, a subexpression can also be optionally followed by an index
        // expression a hash wildcard subexpression by another subexpression
        // and a comparator expression
        m_subexpressionRule = (lit('.')
            >> (m_identifierRule
                | m_multiselectListRule
                | m_multiselectHashRule)[at_c<1>(_val) = _1])
            >> -m_indexExpressionRule[insertNode(*ref(m_rootNode), _1)]
            >> -m_hashWildcardSubexpressionRule[insertNode(*ref(m_rootNode),
                                                           _1)]
            >> -m_subexpressionRule[insertNode(*ref(m_rootNode), _1)]
            >> -m_comparatorExpressionRule[insertNode(*ref(m_rootNode), _1)]
            >> -m_orExpressionRule[insertNode(*ref(m_rootNode), _1)];
        // match a bracket specifier which can be optionally followed by a
        // subexpression a hash wildcard subexpression an index expression
        // and a comparator expression
        m_indexExpressionRule = m_bracketSpecifierRule[at_c<1>(_val) = _1]
            >> -m_subexpressionRule[insertNode(*ref(m_rootNode), _1)]
            >> -m_hashWildcardSubexpressionRule[insertNode(*ref(m_rootNode),
                                                           _1)]
            >> -m_indexExpressionRule[insertNode(*ref(m_rootNode), _1)]
            >> -m_comparatorExpressionRule[insertNode(*ref(m_rootNode), _1)]
            >> -m_orExpressionRule[insertNode(*ref(m_rootNode), _1)];
        // match a slice expression or an array item or a list wildcard or a
        // flatten operator
        m_bracketSpecifierRule = (lit("[")
                                  >> (m_sliceExpressionRule
                                      | m_arrayItemRule
                                      | m_listWildcardRule)
                                  >> lit("]"))
                | m_flattenOperatorRule;
        // match an integer
        m_arrayItemRule = int_;
        // match a pair of square brackets
        m_flattenOperatorRule = eps >> lit("[]");
        // match a colon which can be optionally preceded and followed by a
        // single integer, these matches can also be optionally followed by
        // another colon which can be followed by an integer whose value doesn't
        // equals to 0
        m_sliceExpressionRule = -int_[at_c<0>(_val) = _1]
                >> lit(':')
                >> -int_[at_c<1>(_val) = _1]
                >> -(lit(':') >> -int_[at_c<2>(_val) = _1]);
        // match an asterisk
        m_listWildcardRule = eps >> lit("*");
        // match an asterisk optionally followd by a subexpression an
        // index expression a hash wildcard subexpression and a comparator
        // expression
        m_hashWildcardRule = eps >> lit("*")
            >> -m_subexpressionRule[insertNode(*ref(m_rootNode), _1)]
            >> -m_indexExpressionRule[insertNode(*ref(m_rootNode), _1)]
            >> -m_hashWildcardSubexpressionRule[insertNode(*ref(m_rootNode),
                                                           _1)]
            >> -m_comparatorExpressionRule[insertNode(*ref(m_rootNode), _1)]
            >> -m_orExpressionRule[insertNode(*ref(m_rootNode), _1)];
        // match a dot followd by an asterisk optionally followd by a
        // subexpression an index expression a hash wildcard subexpression
        // and a comparator expression
        m_hashWildcardSubexpressionRule = eps >> lit(".") >> lit("*")
            >> -m_subexpressionRule[insertNode(*ref(m_rootNode), _1)]
            >> -m_indexExpressionRule[insertNode(*ref(m_rootNode), _1)]
            >> -m_hashWildcardSubexpressionRule[insertNode(*ref(m_rootNode),
                                                           _1)]
            >> -m_comparatorExpressionRule[insertNode(*ref(m_rootNode), _1)]
            >> -m_orExpressionRule[insertNode(*ref(m_rootNode), _1)];
        // matches an expression enclosed in square brackets, the expression
        // can optionally be followd by more expressions separated with commas
        m_multiselectListRule = lit('[')
                >> m_expressionRule % lit(',')
                >> lit(']');
        // match a key-value pair enclosed in curly braces, the key-value pair
        // can optionally be followed by more key-value pairs separated with
        // commas
        m_multiselectHashRule = lit('{')
                >> m_keyValuePairRule % lit(',')
                >> lit('}');
        // match an expression preceded by an exclamation mark
        m_notExpressionRule = lit('!') >> m_expressionRule;
        // match a comparator symbol followed by an expression
        m_comparatorExpressionRule = m_comparatorSymbols[at_c<1>(_val) = _1]
                >> m_expressionRule[replaceIfNull(*ref(m_rootNode), _1)];
        // convert textual comparator symbols to enum values
        m_comparatorSymbols.add
            (U"<", ast::ComparatorExpressionNode::Comparator::Less)
            (U"<=", ast::ComparatorExpressionNode::Comparator::LessOrEqual)
            (U"==", ast::ComparatorExpressionNode::Comparator::Equal)
            (U">=", ast::ComparatorExpressionNode::Comparator::GreaterOrEqual)
            (U">", ast::ComparatorExpressionNode::Comparator::Greater)
            (U"!=", ast::ComparatorExpressionNode::Comparator::NotEqual);

        m_orExpressionRule = lit("||")
                >> m_expressionRule[replaceIfNull(*ref(m_rootNode), _1)];

        // match an identifier and an expression separated with a colon
        m_keyValuePairRule = m_identifierRule >> lit(':') >> m_expressionRule;
        // match zero or more literal characters enclosed in grave accents
        m_literalRule = lexeme[ lit('\x60')
                >> *m_literalCharRule[phx::bind(&Grammar::appendUtf8,
                                                this,
                                                at_c<0>(_val),
                                                _1)]
                >> lit('\x60') ];
        // match a character in the range of 0x00-0x5B or 0x5D-0x5F or
        // 0x61-0x10FFFF or a literal escape
        m_literalCharRule = char_(U'\x00', U'\x5B')
                | char_(U'\x5D', U'\x5F')
                | char_(U'\x61', U'\U0010FFFF')
                | m_literalEscapeRule;
        // match a grave accent preceded by an escape or match a backslash if
        // it's not followed by a grave accent
        m_literalEscapeRule = (m_escapeRule
                                >> char_(U'\x60'))
                               | (char_(U'\\')
                                  >> & (!lit('`')));
        // match zero or more raw string characters enclosed in apostrophes
        m_rawStringRule = lexeme[ lit("\'")
                >> *m_rawStringCharRule[phx::bind(&Grammar::appendUtf8,
                                                  this,
                                                  at_c<0>(_val),
                                                  _1)]
                >> lit("\'") ];

        // match a single character in the range of 0x20-0x26 or 0x28-0x5B or
        // 0x5D-0x10FFFF or an escaped apostrophe
        m_rawStringCharRule = (char_(U'\x20', U'\x26')
                               | char_(U'\x28', U'\x5B')
                               | char_(U'\x5D', U'\U0010FFFF'))
                | m_rawStringEscapeRule;

        // match escaped apostrophes
        m_rawStringEscapeRule = m_escapeRule >> char_(U'\'');

        // match unquoted or quoted strings
        m_identifierRule = m_unquotedStringRule | m_quotedStringRule;

        // match a single character in the range of 0x41-0x5A or 0x61-0x7A
        // or 0x5F (A-Za-z_) followed by zero or more characters in the range of
        // 0x30-0x39 (0-9) or 0x41-0x5A (A-Z) or 0x5F (_) or 0x61-0x7A (a-z)
        // and append them to the rule's string attribute encoded as UTF-8
        m_unquotedStringRule
            = lexeme[ ((char_(U'\x41', U'\x5A')
                        | char_(U'\x61', U'\x7A')
                        | char_(U'\x5F'))[phx::bind(&Grammar::appendUtf8,
                                                    this,
                                                    _val,
                                                    _1)]
            >> *(char_(U'\x30', U'\x39')
                 | char_(U'\x41', U'\x5A')
                 | char_(U'\x5F')
                 | char_(U'\x61', U'\x7A'))[phx::bind(&Grammar::appendUtf8,
                                                      this,
                                                      _val,
                                                      _1)]) ];

        // match unescaped or escaped characters enclosed in quotes one or more
        // times and append them to the rule's string attribute encoded as UTF-8
        m_quotedStringRule
            = lexeme[ m_quoteRule
            >> +(m_unescapedCharRule
                 | m_escapedCharRule)[phx::bind(&Grammar::appendUtf8,
                                                this,
                                                _val,
                                                _1)]
            >> m_quoteRule ];

        // match characters in the range of 0x20-0x21 or 0x23-0x5B or
        // 0x5D-0x10FFFF
        m_unescapedCharRule = char_(U'\x20', U'\x21')
            | char_(U'\x23', U'\x5B')
            | char_(U'\x5D', U'\U0010FFFF');

        // match quotation mark literal
        m_quoteRule = lit('\"');

        // match backslash literal
        m_escapeRule = lit('\\');

        // match an escape character followed by quotation mark or backslash or
        // slash or control character or surrogate pair or a single unicode
        // escape
        m_escapedCharRule = lexeme[ m_escapeRule
            >> (char_(U'\"')
                | char_(U'\\')
                | char_(U'/')
                | m_controlCharacterSymbols
                | m_surrogatePairCharacterRule
                | m_unicodeCharRule) ];

        // match a pair of unicode character escapes separated by an escape
        // symbol if the first character's value is between 0xD800-0xDBFF
        // and convert them into a single codepoint
        m_surrogatePairCharacterRule
            = lexeme[ (m_unicodeCharRule >> m_escapeRule >> m_unicodeCharRule)
                [_pass = (_1 >= 0xD800 && _1 <= 0xDBFF),
                _val = phx::bind(&Grammar::parseSurrogatePair,
                                this,
                                _1,
                                _2)] ];

        // match a unicode character escape and convert it into a
        // single codepoint
        m_unicodeCharRule = lexeme[ lit('u')
            >> int_parser<UnicodeChar, 16, 4, 4>() ];

        // convert symbols into control characters
        m_controlCharacterSymbols.add
        (U"b", U'\x08')     // backspace
        (U"f", U'\x0C')     // form feed
        (U"n", U'\x0A')     // line feed
        (U"r", U'\x0D')     // carriage return
        (U"t", U'\x09');    // tab
    }

private:
    qi::rule<Iterator,
             ast::ExpressionNode(),
             qi::locals<ast::ExpressionNode>,
             Skipper > m_expressionRule;
    qi::rule<Iterator,
             ast::SubexpressionNode(),
             Skipper> m_subexpressionRule;
    qi::rule<Iterator,
             ast::IndexExpressionNode(),
             Skipper> m_indexExpressionRule;                
    qi::rule<Iterator,
             ast::HashWildcardNode(),
             Skipper> m_hashWildcardRule;
    qi::rule<Iterator,
             ast::HashWildcardNode(),
             Skipper> m_hashWildcardSubexpressionRule;
    qi::rule<Iterator,
             ast::BracketSpecifierNode(),
             Skipper> m_bracketSpecifierRule;
    qi::rule<Iterator,
             ast::ArrayItemNode(),
             Skipper> m_arrayItemRule;
    qi::rule<Iterator,
             ast::FlattenOperatorNode(),
             Skipper> m_flattenOperatorRule;
    qi::rule<Iterator,
             ast::SliceExpressionNode(),
             Skipper> m_sliceExpressionRule;
    qi::rule<Iterator,
             ast::ListWildcardNode(),
             Skipper> m_listWildcardRule;
    qi::rule<Iterator,
             ast::MultiselectListNode(),
             Skipper> m_multiselectListRule;
    qi::rule<Iterator,
             ast::MultiselectHashNode(),
             Skipper> m_multiselectHashRule;
    qi::rule<Iterator,
             ast::MultiselectHashNode::KeyValuePairType(),
             Skipper> m_keyValuePairRule;
    qi::rule<Iterator, ast::NotExpressionNode(), Skipper> m_notExpressionRule;
    qi::rule<Iterator,
             ast::ComparatorExpressionNode(),
             Skipper> m_comparatorExpressionRule;
    qi::symbols<UnicodeChar,
                ast::ComparatorExpressionNode::Comparator> m_comparatorSymbols;
    qi::rule<Iterator,
             ast::OrExpressionNode(),
             Skipper> m_orExpressionRule;
    qi::rule<Iterator, ast::IdentifierNode(), Skipper> m_identifierRule;
    qi::rule<Iterator, ast::RawStringNode(), Skipper> m_rawStringRule;
    qi::rule<Iterator, ast::LiteralNode(), Skipper> m_literalRule;
    qi::rule<Iterator, UnicodeChar()>       m_literalCharRule;
    qi::rule<Iterator, UnicodeChar()>       m_literalEscapeRule;
    qi::rule<Iterator, UnicodeChar()>       m_rawStringCharRule;
    qi::rule<Iterator, UnicodeChar()>       m_rawStringEscapeRule;
    qi::rule<Iterator, String()>            m_quotedStringRule;
    qi::rule<Iterator, String()>            m_unquotedStringRule;
    qi::rule<Iterator, UnicodeChar()>       m_unescapedCharRule;
    qi::rule<Iterator, UnicodeChar()>       m_escapedCharRule;
    qi::rule<Iterator, UnicodeChar()>       m_unicodeCharRule;
    qi::rule<Iterator, UnicodeChar()>       m_surrogatePairCharacterRule;
    qi::rule<Iterator>                      m_quoteRule;
    qi::rule<Iterator>                      m_escapeRule;
    qi::symbols<UnicodeChar, UnicodeChar>   m_controlCharacterSymbols;
    ast::ExpressionNode* m_rootNode;

    /**
    * @brief Appends the \a utf32Char character to the \a utf8String encoded in
    * UTF-8.
    * @param utf8String The string where the encoded value of the \a utf32Char
    * will be appended.
    * @param utf32Char The input character encoded in UTF-32
    */
    void appendUtf8(String& utf8String, UnicodeChar utf32Char) const
    {
        auto outIt = std::back_inserter(utf8String);
        boost::utf8_output_iterator<decltype(outIt)> utf8OutIt(outIt);
        *utf8OutIt++ = utf32Char;
    }
    /**
     * @brief Parses a surrogate pair character
     * @param highSurrogate High surrogate
     * @param lowSurrogate Low surrogate
     * @return The result of @a highSurrogate and @a lowSurrogate combined
     * into a single codepoint
     */
    UnicodeChar parseSurrogatePair(UnicodeChar const& highSurrogate,
                                   UnicodeChar const& lowSurrogate)
    {
        UnicodeChar unicodeChar = 0x10000;
        unicodeChar += (highSurrogate & 0x03FF) << 10;
        unicodeChar += (lowSurrogate & 0x03FF);
        return unicodeChar;
    }
};
}} // namespace jmespath::parser
#endif // GRAMMAR_H
