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
#include <expected>
#include <functional>
#include <initializer_list>
#include <string_view>
#include <vector>

#include "AST.h"
#include "CompilerError.h"
#include "Expr.h"
#include "Lexer.h"
#include "Stmt.h"
#include "Token.h"

namespace PKIsensee
{

class AbstractSyntaxTree;
class CompilerError;

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
  std::expected<StmtList, CompilerError> GetStatements();
  std::expected<AbstractSyntaxTree, CompilerError> GetAST(); // generate AST

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

  bool IsTokenMatch( TokenType tokenType )
  {
    return Peek().GetType() == tokenType;
  }

  // Determine if the current token matches any of the input tokens and advance if so
  template<typename... TokenTypes>
  bool IsMatchAdvance( TokenTypes... tokenTypes )
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

  // Generic binary expression for plus, minus, multiply and divide
  template<typename... TokenTypes>
  ExprPtr GetBinaryExpr( std::function<ExprPtr()> GetExpr, TokenTypes... tokenTypes )
  {
    ExprPtr lhs = GetExpr();
    while( IsMatchAdvance( tokenTypes... ) )
    {
      Token binaryOp = GetPrevToken();
      ExprPtr rhs = GetExpr();
      lhs = std::make_unique<BinaryExpr>( std::move( lhs ), binaryOp, std::move( rhs ) );
    }
    return lhs;
  }

  Token Consume( TokenType, std::string_view errMsg );

  // In order of precedence from highest to lowest
  ExprPtr GetPrimaryExpr();
  ExprPtr GetFuncCallExpr();
  ExprPtr FinishFuncCallExpr( ExprPtr );
  ExprPtr GetUnaryExpr();
  ExprPtr GetMultiplicationExpr();
  ExprPtr GetAdditionExpr();
  ExprPtr GetComparisonExpr();
  ExprPtr GetEqualityExpr();
  ExprPtr GetAndExpr();
  ExprPtr GetOrExpr();
  ExprPtr GetAssignExpr();
  ExprPtr GetExpr();

  StmtPtr GetExprStmt();
  StmtList GetBlock();
  StmtPtr GetWhileStmt();
  StmtPtr GetReturnStmt();
  StmtPtr GetPrintStmt();
  StmtPtr GetIfStmt();
  StmtPtr GetForStmt();
  StmtPtr GetStmt();
  StmtPtr GetFunc();
  StmtPtr GetVarDecl();
  StmtPtr GetDecl();

private:
  TokenList tokens_;
  size_t currToken_ = 0;

}; // class Parser

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
