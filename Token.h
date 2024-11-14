///////////////////////////////////////////////////////////////////////////////
//
//  Token.h
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

#pragma once
#include <iostream>
#include <string_view>

#include "..\frozen\unordered_map.h"

namespace PKIsensee
{

class Token;

// Update Lexer.cpp with specific values of these tokens

enum class TokenType
{
  // Single-character tokens
  OpenBracket,
  CloseBracket,
  OpenBrace,
  CloseBrace,
  OpenParen,
  CloseParen,
  LessThan,
  GreaterThan,
  EndStatement,
  Assign,
  Plus,
  Minus,
  Multiply,
  Divide,
  Comma,
  Dot,

  // Multi-character tokens
  IsEqual,
  NotEqual,
  LessThanEqual,
  GreaterThanEqual,

  // Literals
  Number,
  Identifier,
  String,

  // Keywords
  And,
  Or,
  Not,
  If,
  Else,
  For,
  While,
  Return,
  True,
  False,
  Print,
  Str,
  Int,
  Char,
  Bool,
  Function,

  // Special tokens
  Invalid,
  EndOfFile,
  Last
};

constexpr size_t kMaxTokenTypes = static_cast<size_t>( TokenType::Last );
constexpr frozen::unordered_map< TokenType, std::string_view, kMaxTokenTypes >
kTokenTypes =
{
  // Single-character tokens
  { TokenType::OpenBracket,      "OpenBracket" },
  { TokenType::CloseBracket,     "CloseBracket" },
  { TokenType::OpenBrace,        "OpenBrace" },
  { TokenType::CloseBrace,       "CloseBrace" },
  { TokenType::OpenParen,        "OpenParen" },
  { TokenType::CloseParen,       "CloseParen" },
  { TokenType::LessThan,         "LessThan" },
  { TokenType::GreaterThan,      "GreaterThan" },
  { TokenType::EndStatement,     "EndStatement" },
  { TokenType::Assign,           "Assign" },
  { TokenType::Plus,             "Plus" },
  { TokenType::Minus,            "Minus" },
  { TokenType::Multiply,         "Multiply" },
  { TokenType::Divide,           "Divide" },
  { TokenType::Comma,            "Comma" },
  { TokenType::Dot,              "Dot" },

  // Multi-character tokens
  { TokenType::IsEqual,          "IsEqual" },
  { TokenType::NotEqual,         "NotEqual" },
  { TokenType::LessThanEqual,    "LessThanEqual" },
  { TokenType::GreaterThanEqual, "GreaterThanEqual" },

  // Literals
  { TokenType::Number,           "Number" },
  { TokenType::Identifier,       "Identifier" },
  { TokenType::String,           "String" },

  // Keywords
  { TokenType::And,              "And" },
  { TokenType::Or,               "Or" },
  { TokenType::Not,              "Not" },
  { TokenType::If,               "If" },
  { TokenType::Else,             "Else" },
  { TokenType::For,              "For" },
  { TokenType::While,            "While" },
  { TokenType::Return,           "Return" },
  { TokenType::True,             "True" },
  { TokenType::False,            "False" },
  { TokenType::Print,            "Print" },
  { TokenType::Str,              "Str" },
  { TokenType::Int,              "Int" },
  { TokenType::Char,             "Char" },
  { TokenType::Bool,             "Bool" },
  { TokenType::Function,         "Function" },

  // Special tokens
  { TokenType::Invalid,          "Invalid" },
  { TokenType::EndOfFile,        "EndOfFile" },
};

///////////////////////////////////////////////////////////////////////////////
//
// Tokens are elements parsed from source code. They have a type (e.g. String,
// Number, CloseParen, etc.) and an associated string_view into the source code

class Token
{
public:
  constexpr Token() = default;
  constexpr Token( const Token& ) = default;
  constexpr Token& operator=( const Token& ) = default;
  constexpr Token( Token&& ) = default;
  constexpr Token& operator=( Token&& ) = default;

  constexpr bool operator==( const Token& other ) const
  {
    return ( type_ == other.type_ ) &&
      ( lexeme_ == other.lexeme_ );
  }

  constexpr Token( TokenType type, std::string_view lexeme ) :
    type_( type ), lexeme_( lexeme )
  {
  }

  constexpr TokenType GetType() const
  {
    return type_;
  }

  constexpr std::string_view GetTypeName() const
  {
    return std::string_view{ kTokenTypes.at( type_ ) };
  }

  constexpr std::string_view GetValue() const
  {
    return lexeme_;
  }

  friend std::ostream& operator<<( std::ostream&, Token );

private:
  TokenType type_ = TokenType::Invalid;
  std::string_view lexeme_; // value
  // TODO add line number

}; // class Token

///////////////////////////////////////////////////////////////////////////////
//
// Stream Token

inline std::ostream& operator<<( std::ostream& out, Token token )
{
  out << token.GetValue();
  if( !token.GetValue().empty() )
    out << ' ';
  out << '[' << token.GetTypeName() << ']';
  return out;
}

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
