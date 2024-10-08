///////////////////////////////////////////////////////////////////////////////
//
//  Lexer.cpp
//
//  Copyright © Pete Isensee (PKIsensee@msn.com).
//  All rights reserved worldwide.
//
//  Permission to copy, modify, reproduce or redistribute this source code is
//  granted provided the above copyright notice is retained in the resulting 
//  source code.
// 
//  This software is provided "as is" and without any express or implied
//  warranties.
//
///////////////////////////////////////////////////////////////////////////////

#include <ranges>

#include "CharUtil.h"
#include "Lexer.h"
#include "Token.h"

using namespace PKIsensee;
namespace ranges = std::ranges;

static constexpr std::array kKeywordTokens = std::to_array<Token>(
  {
    Token{ TokenType::And,    "and" },
    Token{ TokenType::Or,     "or" },
    Token{ TokenType::Not,    "not" },
    Token{ TokenType::Return, "return" },
  } );

static constexpr std::array kOperatorTokens = std::to_array<Token>(
  {
    Token{ TokenType::OpenBracket,      "[" },
    Token{ TokenType::CloseBracket,     "]" },
    Token{ TokenType::OpenBrace,        "{" },
    Token{ TokenType::CloseBrace,       "}" },
    Token{ TokenType::OpenParen,        "(" },
    Token{ TokenType::CloseParen,       ")" },
    Token{ TokenType::LessThan,         "<" },
    Token{ TokenType::GreaterThan,      ">" },
    Token{ TokenType::EndStatement,     ";" },
    Token{ TokenType::Assign,           "=" },
    Token{ TokenType::Plus,             "+" },
    Token{ TokenType::Minus,            "-" },
    Token{ TokenType::Multiply,         "*" },
    Token{ TokenType::Divide,           "/" },
    Token{ TokenType::IsEqual,          "==" },
    Token{ TokenType::NotEqual,         "!=" },
    Token{ TokenType::LessThanEqual,    "<=" },
    Token{ TokenType::GreaterThanEqual, ">=" },
  } );

///////////////////////////////////////////////////////////////////////////////
//
// Extracts the next token from the source code stream

Token Lexer::GetNextToken()
{
  // EOF
  if( curr_ >= source_.end() )
    return Token{ TokenType::EndOfFile, std::string_view{} };

  // Ignore whitespace
  if( CharUtil::IsWhitespace( GetCurrChar() ) )
  {
    while( CharUtil::IsWhitespace( GetCurrChar() ) )
      ++curr_;
  }

  // Identifiers and keywords
  // TODO currently only handles identifiers with alpha chars; doesn't handle 
  // _underscores or namesWithNumbers0123
  const auto idToken = GetLiteralToken( CharUtil::IsAlpha, TokenType::Identifier );
  if( idToken.has_value() )
    return KeywordOrIdentifier( idToken.value() );

  // Numbers
  // TODO currently only handles positive integers; could use regex
  // regex: /^\d+/
  const auto numberToken = GetLiteralToken( CharUtil::IsNumeric, TokenType::Number );
  if( numberToken.has_value() )
    return numberToken.value();

  // Strings
  // regex: /"[^"]*"/
  // regex: /'[^"]*'/
  if( GetCurrChar() == '\'' || GetCurrChar() == '\"' )
  {
    const auto quote = GetCurrChar();
    const std::string_view::const_iterator start = ++curr_; // decl to avoid lint warning
    while( GetCurrChar() != quote )
      ++curr_;

    std::string_view string{ start, curr_ };
    ++curr_;
    return Token{ TokenType::String, string };
  }

  // TODO
  // Single line comments: /^\/\/.*/
  // Multiline comments: /^\/\*[\s\S]*?\*\//

  // Find potential matching operator tokens at this position
  using MatchingTokens = std::vector<Token>; // TODO std::small_vector
  MatchingTokens matchingOperators;
  for( const auto& token : kOperatorTokens )
  {
    if( TokenMatches( token ) )
      matchingOperators.push_back( token );
  }

  // Choose the best (longest) match, e.g. choose "<=" rather than "<"
  const auto it = ranges::find_if( matchingOperators, [&matchingOperators]( const auto& token )
    {
      return ranges::all_of( matchingOperators, [&token]( const auto& other )
        {
          return token.GetValue().size() >= other.GetValue().size();
        } );
    } );
  if( it != std::end( matchingOperators ) )
  {
    curr_ += signed( it->GetValue().size() );
    return *it;
  }

  // Nothing we currently recognize
  return Token{ TokenType::Invalid, std::string_view{ curr_, ++curr_ } };
}

///////////////////////////////////////////////////////////////////////////////
//
// Returns the value of the current character, while correctly handling the
// case where the current character is past the end of the source code

char Lexer::GetCurrChar() const // private
{
  assert( curr_ >= source_.begin() );
  if( curr_ >= source_.end() )
    return '\0';
  return *curr_;
}

///////////////////////////////////////////////////////////////////////////////
//
// Determine if the token at the current position is a match for the incoming
// operator

bool Lexer::TokenMatches( const Token& operatorToken ) const // private
{
  auto start = curr_;
  for( const auto c : operatorToken.GetValue() )
  {
    if( c != *start )
      return false;
    if( ++start >= source_.end() )
      break;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
//
// Determine if the token at the current position is sequential alpha, numeric, 
// or whitespace value

std::optional<Token> Lexer::GetLiteralToken( std::function<bool( char )> charFn,
                                             TokenType tokenType ) // private
{
  if( charFn( GetCurrChar() ) )
  {
    auto start = curr_;
    while( charFn( GetCurrChar() ) )
      ++curr_;

    std::string_view value{ start, curr_ };
    return Token{ tokenType, value };
  }
  return std::nullopt;
}

///////////////////////////////////////////////////////////////////////////////
//
// An identifier might actually be a keyword. Disambiguate and return the
// correct token.

Token Lexer::KeywordOrIdentifier( const Token& token ) const // private
{
  const auto it = ranges::find_if( kKeywordTokens, [&token]( const Token& keyword )
    {
      return token.GetValue() == keyword.GetValue();
    } );
  if( it != std::end( kKeywordTokens ) )
    return *it; // keyword

  return token; // identifier
}

///////////////////////////////////////////////////////////////////////////////
