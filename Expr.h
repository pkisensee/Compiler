///////////////////////////////////////////////////////////////////////////////
//
//  Expr.h
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
#include <memory>

#include "Token.h"

namespace PKIsensee
{

class Expr;
using ExprPtr = std::unique_ptr<Expr>;

///////////////////////////////////////////////////////////////////////////////
//
// Expression abstract base class interface

class Expr
{
public:
  virtual ~Expr() = default;
  Expr() = default;

  // Disable copies, allow moves
  Expr( const Expr& ) = delete;
  Expr& operator=( const Expr& ) = delete;
  Expr( Expr&& ) = default;
  Expr& operator=( Expr&& ) = default;

  virtual void Stream( std::ostream&, uint32_t indent ) const = 0;

}; // class Expr

// TODO consider "final" keyword where appropriate

///////////////////////////////////////////////////////////////////////////////
//
// Unary expression

class UnaryExpr : public Expr
{
public:
  UnaryExpr() = delete;

  UnaryExpr( const Token& unaryOp, ExprPtr expr ) :
    unaryOp_{ unaryOp },
    expr_{ std::move( expr ) }
  {
  }

  // Disable copies, allow moves
  UnaryExpr( const UnaryExpr& ) = delete;
  UnaryExpr& operator=( const UnaryExpr& ) = delete;
  UnaryExpr( UnaryExpr&& ) = default;
  UnaryExpr& operator=( UnaryExpr&& ) = default;

  virtual void Stream( std::ostream&, uint32_t indent ) const final;

private:
  Token unaryOp_;
  ExprPtr expr_;

}; // class UnaryExpr

///////////////////////////////////////////////////////////////////////////////
//
// Binary expression

class BinaryExpr : public Expr
{
public:
  BinaryExpr() = delete;

  BinaryExpr( ExprPtr leftExpr, const Token& binaryOp, ExprPtr rightExpr ) :
    leftExpr_{ std::move( leftExpr ) },
    binaryOp_{ binaryOp },
    rightExpr_{ std::move( rightExpr ) }
  {
  }

  // Disable copies, allow moves
  BinaryExpr( const BinaryExpr& ) = delete;
  BinaryExpr& operator=( const BinaryExpr& ) = delete;
  BinaryExpr( BinaryExpr&& ) = default;
  BinaryExpr& operator=( BinaryExpr&& ) = default;

  virtual void Stream( std::ostream&, uint32_t indent ) const final;

private:
  ExprPtr leftExpr_;
  Token binaryOp_;
  ExprPtr rightExpr_;

}; // class BinaryExpr

///////////////////////////////////////////////////////////////////////////////
//
// Literal expression

class LiteralExpr : public Expr
{
public:
  LiteralExpr() = delete;

  explicit LiteralExpr( const Token& token ) :
    literal_{ token }
  {
  }

  // Disable copies, allow moves
  LiteralExpr( const LiteralExpr& ) = delete;
  LiteralExpr& operator=( const LiteralExpr& ) = delete;
  LiteralExpr( LiteralExpr&& ) = default;
  LiteralExpr& operator=( LiteralExpr&& ) = default;

  virtual void Stream( std::ostream&, uint32_t indent ) const final;

private:
  Token literal_;

}; // class LiteralExpr

///////////////////////////////////////////////////////////////////////////////
//
// Parenthesized expression

class ParensExpr : public Expr
{
public:
  ParensExpr() = delete;

  explicit ParensExpr( ExprPtr expr ) :
    expr_( std::move( expr ) )
  {
  }

  // Disable copies, allow moves
  ParensExpr( const ParensExpr& ) = delete;
  ParensExpr& operator=( const ParensExpr& ) = delete;
  ParensExpr( ParensExpr&& ) = default;
  ParensExpr& operator=( ParensExpr&& ) = default;

  virtual void Stream( std::ostream&, uint32_t indent ) const final;

private:
  ExprPtr expr_;

}; // class ParensExpr

} // namespace PKIsensee

////////////////////////////////////////////////////////////////////////////////////////////////////
