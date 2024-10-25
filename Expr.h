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
#include <memory>
#include <vector>

#include "Value.h"

namespace PKIsensee
{

class Token;
class Expr;
using ExprPtr = std::unique_ptr<Expr>; // TODO consider shared_ptr to avoid unreadable std::moves
using ExprList = std::vector<ExprPtr>;

///////////////////////////////////////////////////////////////////////////////
//
// Visitor interface used to walk expression tree to evaluate result.
// Designed as a mix-in base class.

class LiteralExpr;
class UnaryExpr;
class BinaryExpr;
class ParensExpr;
class AssignExpr;
class LogicalExpr;
class VarExpr;
class FuncExpr;

template<typename Result>
class ExprEvaluator
{
public:
  virtual ~ExprEvaluator() = default;
  ExprEvaluator() = default;
  ExprEvaluator( const ExprEvaluator& ) = default;
  ExprEvaluator& operator=( const ExprEvaluator& ) = default;
  ExprEvaluator( ExprEvaluator&& ) = default;
  ExprEvaluator& operator=( ExprEvaluator&& ) = default;

  virtual Result EvalLiteralExpr( const LiteralExpr& ) const = 0;
  virtual Result EvalUnaryExpr( const UnaryExpr& ) const = 0;
  virtual Result EvalBinaryExpr( const BinaryExpr& ) const = 0;
  virtual Result EvalParensExpr( const ParensExpr& ) const = 0;
  virtual Result EvalAssignExpr( const AssignExpr& ) const = 0;
  virtual Result EvalLogicalExpr( const LogicalExpr& ) const = 0;
  virtual Result EvalVarExpr( const VarExpr& ) const = 0;
  virtual Result EvalFuncExpr( const FuncExpr& ) const = 0;
};

///////////////////////////////////////////////////////////////////////////////
//
// Visitor interface used to walk expression tree to output the tree.
// Designed as a mix-in base class.

class ExprStreamer
{
public:
  virtual ~ExprStreamer() = default;
  ExprStreamer() = default;
  ExprStreamer( const ExprStreamer& ) = default;
  ExprStreamer& operator=( const ExprStreamer& ) = default;
  ExprStreamer( ExprStreamer&& ) = default;
  ExprStreamer& operator=( ExprStreamer&& ) = default;

  virtual void StreamLiteralExpr( const LiteralExpr&, uint32_t indent ) const = 0;
  virtual void StreamUnaryExpr( const UnaryExpr&, uint32_t indent ) const = 0;
  virtual void StreamBinaryExpr( const BinaryExpr&, uint32_t indent ) const = 0;
  virtual void StreamParensExpr( const ParensExpr&, uint32_t indent ) const = 0;
  virtual void StreamAssignExpr( const AssignExpr&, uint32_t indent ) const = 0;
  virtual void StreamLogicalExpr( const LogicalExpr&, uint32_t indent ) const = 0;
  virtual void StreamVarExpr( const VarExpr&, uint32_t indent ) const = 0;
  virtual void StreamFuncExpr( const FuncExpr&, uint32_t indent ) const = 0;
};

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

  virtual Value Eval( const ExprEvaluator<Value>& ) const = 0;
  virtual void Stream( const ExprStreamer&, uint32_t indent ) const = 0;

}; // class Expr

///////////////////////////////////////////////////////////////////////////////
//
// Literal expression

class LiteralExpr : public Expr
{
public:
  LiteralExpr() = delete;

  explicit LiteralExpr( Token token ) :
    literal_{ token }
  {
  }

  explicit LiteralExpr( const Value& value ) :
    literal_{ value }
  {
  }

  // Disable copies, allow moves
  LiteralExpr( const LiteralExpr& ) = delete;
  LiteralExpr& operator=( const LiteralExpr& ) = delete;
  LiteralExpr( LiteralExpr&& ) = default;
  LiteralExpr& operator=( LiteralExpr&& ) = default;

  Value GetLiteral() const
  {
    return literal_;
  }

  virtual Value Eval( const ExprEvaluator<Value>& ) const override final;
  virtual void Stream( const ExprStreamer&, uint32_t indent ) const override final;

private:
  Value literal_;

}; // class LiteralExpr

///////////////////////////////////////////////////////////////////////////////
//
// Unary expression

class UnaryExpr : public Expr
{
public:
  UnaryExpr() = delete;

  UnaryExpr( Token unaryOp, ExprPtr expr ) :
    unaryOp_{ unaryOp },
    expr_{ std::move( expr ) }
  {
  }

  // Disable copies, allow moves
  UnaryExpr( const UnaryExpr& ) = delete;
  UnaryExpr& operator=( const UnaryExpr& ) = delete;
  UnaryExpr( UnaryExpr&& ) = default;
  UnaryExpr& operator=( UnaryExpr&& ) = default;

  const Expr& GetExpr() const
  {
    return *expr_;
  }

  Token GetUnaryOp() const
  {
    return unaryOp_;
  }

  virtual Value Eval( const ExprEvaluator<Value>& ) const override final;
  virtual void Stream( const ExprStreamer&, uint32_t indent ) const override final;

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

  BinaryExpr( ExprPtr leftExpr, Token binaryOp, ExprPtr rightExpr ) :
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

  const Expr& GetLeftExpr() const
  {
    return *leftExpr_;
  }

  const Expr& GetRightExpr() const
  {
    return *rightExpr_;
  }

  Token GetBinaryOp() const
  {
    return binaryOp_;
  }

  virtual Value Eval( const ExprEvaluator<Value>& ) const override final;
  virtual void Stream( const ExprStreamer&, uint32_t indent ) const override final;

private:
  ExprPtr leftExpr_;
  Token binaryOp_;
  ExprPtr rightExpr_;

}; // class BinaryExpr

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

  const Expr& GetExpr() const
  {
    return *expr_;
  }

  virtual Value Eval( const ExprEvaluator<Value>& ) const override final;
  virtual void Stream( const ExprStreamer&, uint32_t indent ) const override final;

private:
  ExprPtr expr_;

}; // class ParensExpr

///////////////////////////////////////////////////////////////////////////////
//
// Assignment expression

class AssignExpr : public Expr
{
public:
  AssignExpr() = delete;

  AssignExpr( Token lhsVariable, ExprPtr rhsValue ) :
    lhsVariable_( lhsVariable ),
    rhsValue_( std::move(rhsValue) )
  {
  }

  // Disable copies, allow moves
  AssignExpr( const AssignExpr& ) = delete;
  AssignExpr& operator=( const AssignExpr& ) = delete;
  AssignExpr( AssignExpr&& ) = default;
  AssignExpr& operator=( AssignExpr&& ) = default;

  const Expr& GetValue() const
  {
    return *rhsValue_;
  }

  virtual Value Eval( const ExprEvaluator<Value>& ) const override final;
  virtual void Stream( const ExprStreamer&, uint32_t indent ) const override final;

private:
  Token lhsVariable_;
  ExprPtr rhsValue_;

}; // class AssignExpr

///////////////////////////////////////////////////////////////////////////////
//
// Logical expression

class LogicalExpr : public Expr
{
public:
  LogicalExpr() = delete;

  LogicalExpr( ExprPtr lhs, Token logicalOp, ExprPtr rhs ) :
    leftExpr_{ std::move(lhs) },
    logicalOp_{ logicalOp },
    rightExpr_{ std::move(rhs) }
  {
  }

  // Disable copies, allow moves
  LogicalExpr( const LogicalExpr& ) = delete;
  LogicalExpr& operator=( const LogicalExpr& ) = delete;
  LogicalExpr( LogicalExpr&& ) = default;
  LogicalExpr& operator=( LogicalExpr&& ) = default;

  const Expr& GetLeftExpr() const
  {
    return *leftExpr_;
  }

  const Expr& GetRightExpr() const
  {
    return *rightExpr_;
  }

  Token GetLogicalOp() const
  {
    return logicalOp_;
  }

  virtual Value Eval( const ExprEvaluator<Value>& ) const override final;
  virtual void Stream( const ExprStreamer&, uint32_t indent ) const override final;

private:
  ExprPtr leftExpr_;
  Token logicalOp_;
  ExprPtr rightExpr_;

}; // class LogicalExpr TODO merge with BinaryExpr?

///////////////////////////////////////////////////////////////////////////////
//
// Variable expression

class VarExpr : public Expr
{
public:
  VarExpr() = delete;

  explicit VarExpr( Token variable ) :
    variable_{ variable }
  {
  }

  // Disable copies, allow moves
  VarExpr( const VarExpr& ) = delete;
  VarExpr& operator=( const VarExpr& ) = delete;
  VarExpr( VarExpr&& ) = default;
  VarExpr& operator=( VarExpr&& ) = default;

  Token GetVariable() const
  {
    return variable_;
  }

  virtual Value Eval( const ExprEvaluator<Value>& ) const override final;
  virtual void Stream( const ExprStreamer&, uint32_t indent ) const override final;

private:
  Token variable_;

}; // class VarExpr

///////////////////////////////////////////////////////////////////////////////
//
// Function expression

class FuncExpr : public Expr
{
public:
  FuncExpr() = delete;

  FuncExpr( ExprPtr fnName, ExprList arguments ) :
    fnName_{ std::move(fnName) },
    arguments_{ std::move(arguments) }
  {
  }

  // Disable copies, allow moves
  FuncExpr( const FuncExpr& ) = delete;
  FuncExpr& operator=( const FuncExpr& ) = delete;
  FuncExpr( FuncExpr&& ) = default;
  FuncExpr& operator=( FuncExpr&& ) = default;

  const Expr& GetFuncName() const
  {
    return *fnName_;
  }

  const ExprList& GetArgs() const
  {
    return arguments_;
  }

  virtual Value Eval( const ExprEvaluator<Value>& ) const override final;
  virtual void Stream( const ExprStreamer&, uint32_t indent ) const override final;

private:
  ExprPtr fnName_;
  ExprList arguments_;

}; // class FuncExpr

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
