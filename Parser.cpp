///////////////////////////////////////////////////////////////////////////////
//
//  Parser.cpp
//
//  Copyright � Pete Isensee (PKIsensee@msn.com).
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
// Checks to see if the token at the current position matches one of the
// incoming token types

bool Parser::IsMatch( std::initializer_list<TokenType> tokenTypes ) // private
{
  for( const auto& tokenType : tokenTypes )
  {
    if( IsTokenMatch( tokenType ) )
    {
      Advance();
      return true;
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
//
// If the token at the current position matches the incoming type, advance
// to the next token

Token Parser::Consume( TokenType tokenType ) // private
{
  if( IsTokenMatch( tokenType ) )
    return Advance();

  // throw error with message
  assert( false );
  return {};
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract the expression at the highest level of precedence
// 
// Grammar: literal | grouping

ExprPtr Parser::GetPrimaryExpr()
{
  // if (IsMatch(TokenType::False)) return new LiteralExpr(false);
  // if (IsMatch(TokenType::True))  return new LiteralExpr(true);
  // if (IsMatch(TokenType::Null))  return new LiteralExpr(nullptr);

  if( IsMatch( { TokenType::Number, TokenType::String } ) )
    return std::make_unique<LiteralExpr>( GetPrevToken() );

  if( IsMatch( { TokenType::OpenParen } ) )
  {
    ExprPtr expr = GetExpr();
    Consume( TokenType::CloseParen );
    return std::make_unique<GroupingExpr>( std::move( expr ) );
  }

  // error "expecting expression"
  assert( false );
  return {};
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract a unary expression
// 
// Grammar: ('!' | '-') unary-expr | primary-expr ;

ExprPtr Parser::GetUnaryExpr()
{
  if( IsMatch( { TokenType::Not, TokenType::Minus } ) )
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
  ExprPtr lhs = GetUnaryExpr();
  while( IsMatch( { TokenType::Multiply, TokenType::Divide } ) )
  {
    Token multiplyOp = GetPrevToken();
    ExprPtr rhs = GetUnaryExpr();
    lhs = std::make_unique<BinaryExpr>( std::move( lhs ), multiplyOp, std::move( rhs ) );
  }
  return lhs;
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract an addition or subtraction expression
// 
// Grammar: multiply-expr ('+' | '-') multiply-expr

ExprPtr Parser::GetAdditionExpr()
{
  ExprPtr lhs = GetMultiplicationExpr();
  while( IsMatch( { TokenType::Plus, TokenType::Minus } ) )
  {
    Token additionOp = GetPrevToken();
    ExprPtr rhs = GetMultiplicationExpr();
    lhs = std::make_unique<BinaryExpr>( std::move( lhs ), additionOp, std::move( rhs ) );
  }
  return lhs;
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract a comparison expression
// 
// Grammar: add-expr ('<' | '<=' | '>' | '>=' ) add-expr

ExprPtr Parser::GetComparisonExpr()
{
  ExprPtr lhs = GetAdditionExpr(); // TODO prefer lhs/rhs?
  while( IsMatch( { TokenType::GreaterThan,
                    TokenType::GreaterThanEqual,
                    TokenType::LessThan,
                    TokenType::LessThanEqual } ) )
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
  while( IsMatch( { TokenType::NotEqual, TokenType::IsEqual } ) )
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
//
// Stream expressions for debugging

void Indent( std::ostream& out, uint32_t indent )
{
  for( uint32_t i = 0u; i < indent; ++i )
    out << "  ";
}

void UnaryExpr::Stream( std::ostream& out, uint32_t indent ) const // virtual
{
  Indent( out, indent );
  out << unaryOp_ << '\n';
  expr_->Stream( out, indent + 1 );
}

void BinaryExpr::Stream( std::ostream& out, uint32_t indent ) const // virtual
{
  Indent( out, indent );
  out << binaryOp_ << '\n';
  leftExpr_->Stream( out, indent + 1 );
  rightExpr_->Stream( out, indent + 1 );
}

void LiteralExpr::Stream( std::ostream& out, uint32_t indent ) const // virtual
{
  Indent( out, indent );
  out << literal_ << '\n';
}

void GroupingExpr::Stream( std::ostream& out, uint32_t indent ) const // virtual
{
  expr_->Stream( out, indent );
}

///////////////////////////////////////////////////////////////////////////////
