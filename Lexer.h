///////////////////////////////////////////////////////////////////////////////
//
//  Lexer.h
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
#include <functional>
#include <optional>
#include <string_view>

#include "Token.h"

namespace PKIsensee
{

using TokenList = std::vector<Token>;

///////////////////////////////////////////////////////////////////////////////
//
// The Lexer extracts the tokens (i.e. words) in a source file
//
// Usage:
//    Lexer lexer( sourceCode );
//    lexer.ExtractTokens(); // may throw
//    for( const auto& token : lexer.GetTokens() )
//      ...

class Lexer
{
public:
  Lexer( std::string_view source ) :
    source_{ source },
    start_{ source.begin() },
    curr_{ source.begin() }
  {
  }

  void ExtractTokens(); // may throw CompilerError

  const TokenList& GetTokens() const
  {
    return tokens_;
  }

private:

  bool IsAtEnd() const
  {
    return curr_ >= source_.end();
  }

  char Peek() const
  {
    return IsAtEnd() ? '\0' : *curr_;
  }

  void ExtractToken();
  void AddToken( TokenType );
  char Advance();
  bool IsMatchAdvance( char );
  char PeekNext() const;
  void SkipComment();
  void AddStringToken( char );
  void AddNumberToken();
  void AddIdentifierToken();

private:
  std::string_view source_;
  std::string_view::const_iterator start_;
  std::string_view::const_iterator curr_;
  uint32_t line_ = 1;
  TokenList tokens_;

}; // class Lexer

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
