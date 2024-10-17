///////////////////////////////////////////////////////////////////////////////
//
//  Parser.cpp
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
//  Derived from Crafting Interpreters by Robert Nystrom
//  https://craftinginterpreters.com/
//
///////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <iostream>
#include <ranges>

#include "CompilerError.h"
#include "Lexer.h"
#include "Parser.h"

using namespace PKIsensee;
namespace ranges = std::ranges;

///////////////////////////////////////////////////////////////////////////////
//
// Parse the incoming stream into tokens

void Parser::Parse( std::string_view source )
{
  std::cout << "Parse: " << source << '\n';
  tokens_.resize( 0 );
  Lexer lexer( source );
  for( ;;)
  {
    Token token = lexer.GetNextToken();
    tokens_.push_back( token );
    if( token.GetType() == TokenType::EndOfFile )
      return;
  }
}

///////////////////////////////////////////////////////////////////////////////
//
// True if all tokens are valid

bool Parser::AllTokensValid() const
{
  return ranges::all_of( tokens_, []( const auto& token )
    {
      return token.GetType() != TokenType::Invalid;
    } );
}

///////////////////////////////////////////////////////////////////////////////
//
// If the token at the current position matches the incoming type, advance
// to the next token

Token Parser::Consume( TokenType tokenType, std::string_view errMsg ) // private
{
  if( IsTokenMatch( tokenType ) )
    return Advance();

  throw CompilerError{ errMsg, Peek() };
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract the expression at the highest level of precedence
// 
// Grammar: literal | grouping

ExprPtr Parser::GetPrimaryExpr()
{
  if (IsMatch( TokenType::Number, TokenType::String, 
               TokenType::True, TokenType::False ) )
    return std::make_unique<LiteralExpr>( GetPrevToken() );

  if( IsMatch( TokenType::OpenParen ) )
  {
    ExprPtr expr = GetExpr();
    Consume( TokenType::CloseParen, "Expecting ')' after expression" );
    return std::make_unique<ParensExpr>( std::move( expr ) );
  }

  throw CompilerError{ "Parentheses don't match", Peek() };
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract a unary expression
// 
// Grammar: ('!' | '-') unary-expr | primary-expr ;

ExprPtr Parser::GetUnaryExpr()
{
  if( IsMatch( TokenType::Not, TokenType::Minus ) )
  {
    Token unaryOp = GetPrevToken();
    ExprPtr unaryExpr = GetUnaryExpr();
    return std::make_unique<UnaryExpr>( unaryOp, std::move( unaryExpr ) );
  }
  return GetPrimaryExpr();
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract a multiply or division expression
// 
// Grammar: unary-expr ('*' | '/') unary-expr

ExprPtr Parser::GetMultiplicationExpr()
{
  auto exprFn = std::bind( &Parser::GetUnaryExpr, this );
  return GetBinaryExpr( exprFn, TokenType::Multiply, TokenType::Divide );
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract an addition or subtraction expression
// 
// Grammar: multiply-expr ('+' | '-') multiply-expr

ExprPtr Parser::GetAdditionExpr()
{
  auto exprFn = std::bind( &Parser::GetMultiplicationExpr, this );
  return GetBinaryExpr( exprFn, TokenType::Plus, TokenType::Minus );
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract a comparison expression
// 
// Grammar: add-expr ('<' | '<=' | '>' | '>=' ) add-expr

ExprPtr Parser::GetComparisonExpr()
{
  ExprPtr lhs = GetAdditionExpr();
  while( IsMatch( TokenType::GreaterThan,
                  TokenType::GreaterThanEqual,
                  TokenType::LessThan,
                  TokenType::LessThanEqual ) )
  {
    Token compOp = GetPrevToken();
    ExprPtr rhs = GetAdditionExpr();
    lhs = std::make_unique<BinaryExpr>( std::move( lhs ), compOp, std::move( rhs ) );
  }
  return lhs;
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract an equal or not-equal expression
// 
// Grammar: comp-expr ('==' | '!=' ) comp-expr

ExprPtr Parser::GetEqualityExpr()
{
  ExprPtr lhs = GetComparisonExpr();
  while( IsMatch( TokenType::NotEqual, TokenType::IsEqual ) )
  {
    Token compOp = GetPrevToken();
    ExprPtr rhs = GetComparisonExpr();
    lhs = std::make_unique<BinaryExpr>( std::move( lhs ), compOp, std::move( rhs ) );
  }
  return lhs;
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract an expression at any precedence level. Since equality has the lowest
// precedence, it covers everything else.
// 
// Grammar: equality-expr

ExprPtr Parser::GetExpr()
{
  return GetEqualityExpr();
}

///////////////////////////////////////////////////////////////////////////////
//
// Stream token data as text for debugging

std::ostream& PKIsensee::operator<<( std::ostream& out, const Parser& parser )
{
  for( const auto& token : parser.tokens_ )
    out << token << '\n';
  return out;
}

///////////////////////////////////////////////////////////////////////////////
