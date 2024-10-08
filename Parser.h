///////////////////////////////////////////////////////////////////////////////
//
//  Parser.h
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
#include <exception>
#include <expected>
#include <initializer_list>
#include <vector>

#include "AST.h"
#include "CompilerError.h"
#include "Expr.h"
#include "Token.h"

namespace PKIsensee
{

///////////////////////////////////////////////////////////////////////////////
//
// The Parser identifies the construction (i.e. sentences, paragraphs) of 
// the source file, producing a list of tokens, which in turn can be extracted
// into an abstract syntax tree (AST).

class Parser
{
public:
  Parser() = default;

  // Disable copy/move
  Parser( const Parser& ) = delete;
  Parser( Parser&& ) = delete;
  Parser& operator=( const Parser& ) = delete;
  Parser& operator=( Parser&& ) = delete;

  void Parse( std::string_view ); // generate token list

  std::expected<AbstractSyntaxTree, CompilerError> GetAST() // generate AST
  {
    currToken_ = 0;
    try
    {
      AbstractSyntaxTree ast{ GetExpr() };
      return ast;
    }
    catch( CompilerError& e )
    {
      return std::unexpected( e );
    }
  }

  size_t GetTokenCount() const
  {
    return tokens_.size();
  }

  Token GetToken( size_t index ) const
  {
    assert( index < tokens_.size() );
    return tokens_[index];
  }

  bool AllTokensValid() const;

  friend std::ostream& operator<<( std::ostream&, const Parser& );

private:

  Token Peek()
  {
    return tokens_[currToken_];
  }

  Token GetPrevToken()
  {
    assert( currToken_ != 0 );
    return tokens_[currToken_ - 1];
  }

  Token Advance()
  {
    if( Peek().GetType() != TokenType::EndOfFile )
      ++currToken_;
    return GetPrevToken();
  }

  bool IsTokenMatch( TokenType tokenType ) // check
  {
    // TODO need to check for Peek().GetType() == EOF?
    return Peek().GetType() == tokenType;
  }

  template<typename... TokenTypes>
  bool IsMatch( TokenTypes... tokenTypes )
  {
    std::initializer_list<TokenType> tokenTypeList{ tokenTypes... };
    for( const auto& tokenType : tokenTypeList )
    {
      if( IsTokenMatch( tokenType ) )
      {
        Advance();
        return true;
      }
    }
    return false;
  }

  Token Consume( TokenType );

  // In order of precedence from highest to lowest
  ExprPtr GetPrimaryExpr();
  ExprPtr GetUnaryExpr();
  ExprPtr GetMultiplicationExpr();
  ExprPtr GetAdditionExpr();
  ExprPtr GetComparisonExpr();
  ExprPtr GetEqualityExpr();
  ExprPtr GetExpr();

private:
  std::vector<Token> tokens_;
  size_t currToken_ = 0;

}; // class Parser

} // namespace PKIsensee

////////////////////////////////////////////////////////////////////////////////////////////////////

