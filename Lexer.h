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

///////////////////////////////////////////////////////////////////////////////
//
// The Lexer extracts the tokens (i.e. words) in a source file

class Lexer
{
public:
  Lexer( std::string_view source ) :
    source_{ source },
    curr_{ source.begin() }
  {
  }

  Token GetNextToken();

private:
  char GetCurrChar() const;
  std::optional<Token> GetLiteralToken( std::function<bool( char )>, TokenType );
  bool TokenMatches( const Token& operatorOrKeyword ) const;
  Token KeywordOrIdentifier( const Token& ) const;

private:
  std::string_view source_;
  std::string_view::const_iterator curr_;

}; // class Lexer

} // namespace PKIsensee

////////////////////////////////////////////////////////////////////////////////////////////////////

