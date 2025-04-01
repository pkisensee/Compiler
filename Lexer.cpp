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
#include "CompilerError.h"
#include "Lexer.h"
#include "Token.h"

using namespace PKIsensee;
namespace ranges = std::ranges;

static constexpr std::array kKeywordTokens = std::to_array<Token>(
  {
    Token{ TokenType::And,       "and" },
    Token{ TokenType::Or,        "or" },
    Token{ TokenType::Not,       "not" },
    Token{ TokenType::If,        "if" },
    Token{ TokenType::Else,      "else" },
    Token{ TokenType::For,       "for" },
    Token{ TokenType::While,     "while" },
    Token{ TokenType::Return,    "return" },
    Token{ TokenType::True,      "true" },
    Token{ TokenType::False,     "false" },
    Token{ TokenType::Print,     "print" },
    Token{ TokenType::Str,       "str" },
    Token{ TokenType::Int,       "int" },
    Token{ TokenType::Char,      "char" },
    Token{ TokenType::Bool,      "bool" },
    Token{ TokenType::Function,  "fun" },
  } );

///////////////////////////////////////////////////////////////////////////////
//
// Turn the source code into a list of tokens

void Lexer::ExtractTokens()
{
  tokens_.clear();
  start_ = curr_ = source_.begin();
  while( !IsAtEnd() )
    ExtractToken();
  start_ = curr_ = source_.end();
  AddToken( TokenType::EndOfFile );
}

///////////////////////////////////////////////////////////////////////////////
//
// Pull out an individual token or skip non-tokens such as whitespace and comments
//
// start_ marks the beginning of the potential token
// curr_ marks the current character, always >= start_
// When a token is recognized, the lexeme is given by: [start_, curr_)

void Lexer::ExtractToken() // private
{
  start_ = curr_;
  char c = Advance();
  switch( c )
  {
  // Single characters
  case '[': AddToken( TokenType::OpenBracket ); break;
  case ']': AddToken( TokenType::CloseBracket ); break;
  case '{': AddToken( TokenType::OpenBrace ); break;
  case '}': AddToken( TokenType::CloseBrace ); break;
  case '(': AddToken( TokenType::OpenParen ); break;
  case ')': AddToken( TokenType::CloseParen ); break;
  case ';': AddToken( TokenType::EndStatement ); break;
  case '+': AddToken( TokenType::Plus ); break;
  case '-': AddToken( TokenType::Minus ); break;
  case '*': AddToken( TokenType::Multiply ); break;
  case '%': AddToken( TokenType::Modulus ); break;
  case ',': AddToken( TokenType::Comma ); break;
  case '.': AddToken( TokenType::Dot ); break;

  // Single/multi character tokens, e.g. < or <=
  case '!': AddToken( IsMatchAdvance('=') ? TokenType::NotEqual : 
                                            TokenType::Not ); break;
  case '=': AddToken( IsMatchAdvance('=') ? TokenType::IsEqual : 
                                            TokenType::Assign ); break;
  case '<': AddToken( IsMatchAdvance('=') ? TokenType::LessThanEqual : 
                                            TokenType::LessThan ); break;
  case '>': AddToken( IsMatchAdvance('=') ? TokenType::GreaterThanEqual : 
                                            TokenType::GreaterThan ); break;
  case '/':
    if( IsMatchAdvance( '/' ) ) // double slash
      SkipComment();
    else // single slash
      AddToken( TokenType::Divide );
    break;
  case ' ': // ignore whitespace
  case '\r':
  case '\t':
    break;
  case '\n': // new line
    ++line_;
    break;
  case '\"': // double or single quotes mark string literals
  case '\'':
    AddStringToken( c ); 
    break;
  default:
    if( CharUtil::IsDigit( c ) )
      AddNumberToken();
    else if( CharUtil::IsAlpha( c ) || c == '_' )
      AddIdentifierToken();
    else
      // TODO create catch block in the appropriate spot
      throw CompilerError{ std::format( "Unexpected character '{}' on line {}", c, line_ ) };
    break;
  }
}

///////////////////////////////////////////////////////////////////////////////
//
// Found a token; add it to the list

void Lexer::AddToken( TokenType tokenType ) // private
{
  tokens_.emplace_back( tokenType, std::string_view{ start_, curr_ } );
}

///////////////////////////////////////////////////////////////////////////////
//
// Get the current character and advance to the next

char Lexer::Advance() // private
{
  char prev = *curr_;
  ++curr_;
  return prev;
}

///////////////////////////////////////////////////////////////////////////////
//
// If the current character matches the expected value, then advance

bool Lexer::IsMatchAdvance( char expected ) // private
{
  if( IsAtEnd() )
    return false;
  if( *curr_ != expected )
    return false;

  ++curr_;
  return true;
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract the value of the next character without advancing

char Lexer::PeekNext() const // private
{
  auto next = curr_ + 1;
  if( next >= source_.end() )
    return '\0';
  return *next;
}

///////////////////////////////////////////////////////////////////////////////
//
// Skip comments, which run to the end of the line

void Lexer::SkipComment() // private
{
  while( Peek() != '\n' && !IsAtEnd() )
    Advance();
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract a string literal, which is delimited by quotes

void Lexer::AddStringToken( char quote ) // private
{
  while( Peek() != quote && !IsAtEnd() )
  {
    if( Peek() == '\n' )
      ++line_;
    Advance();
  }

  if( IsAtEnd() )
  {
    Token incomplete{ TokenType::String, std::string_view{start_, curr_} };
    throw CompilerError{ "Unterminated string", incomplete };
  }

  ++start_; // lexeme doesn't include the opening quote
  AddToken( TokenType::String );
  Advance(); // lexeme doesn't include the closing quote
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract a numeric literal, including any embedded decimal point

void Lexer::AddNumberToken() // private
{
  while( CharUtil::IsDigit( Peek() ) )
    Advance();
  if( Peek() == '.' && CharUtil::IsDigit( PeekNext() ) )
  {
    Advance(); // eat the decimal point
    while( CharUtil::IsDigit( Peek() ) )
      Advance();
  }
  AddToken( TokenType::Number );
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract an identifier or keyword

void Lexer::AddIdentifierToken() // private
{
  while( CharUtil::IsAlphaNum( Peek() ) || Peek() == '_' )
    Advance();

  // If the string is a keyword, it's a special token type
  std::string_view identifier{ start_, curr_ };
  const auto it = ranges::find_if( kKeywordTokens, [&identifier]( Token keyword )
    {
      return identifier == keyword.GetValue();
    } );
  AddToken( it != std::end( kKeywordTokens ) ? it->GetType() : TokenType::Identifier );
}

///////////////////////////////////////////////////////////////////////////////
