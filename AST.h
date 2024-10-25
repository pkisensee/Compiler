///////////////////////////////////////////////////////////////////////////////
//
//  AST.h
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
#include "Expr.h"

namespace PKIsensee
{

///////////////////////////////////////////////////////////////////////////////
//
// Representation of source code
// 
// Given the expression: 1 + 2 * 3
//
// The resulting AST is:
//    +
//   / \
//  1   *
//     / \
//    2   3
//
// operator<<() expresses the AST in sideways form, where each 
// indentation is a new level in the tree
//
// + [Plus]
//   1 [Number]
//   * [Multiply]
//     2 [Number]
//     3 [Number]

class AbstractSyntaxTree : public ExprStreamer
{
public:
  AbstractSyntaxTree() = delete;

  explicit AbstractSyntaxTree( ExprPtr expr ) :
    root_( std::move(expr) )
  {
  }

  // Disable copies, allow moves
  AbstractSyntaxTree( const AbstractSyntaxTree& ) = delete;
  AbstractSyntaxTree& operator=( const AbstractSyntaxTree& ) = delete;
  AbstractSyntaxTree( AbstractSyntaxTree&& ) = default;
  AbstractSyntaxTree& operator=( AbstractSyntaxTree&& ) = default;

  const Expr& GetRoot() const
  {
    return *root_;
  }

  friend std::ostream& operator<<( std::ostream&, const AbstractSyntaxTree& );

private:

  void Stream() const
  {
    GetRoot().Stream( *this, 0 ); // dispatch to appropriate virtual fn
  }

  void Indent( uint32_t indent ) const
  {
    for( uint32_t i = 0u; i < indent; ++i )
      *out_ << "  ";
  }

  // ExprStreamer overrides
  virtual void StreamUnaryExpr( const UnaryExpr& expr, uint32_t indent ) const override final
  {
    Indent( indent );
    *out_ << expr.GetUnaryOp() << '\n';
    expr.Stream( *this, indent + 1 );
  }

  virtual void StreamBinaryExpr( const BinaryExpr& expr, uint32_t indent ) const override final
  {
    Indent( indent );
    *out_ << expr.GetBinaryOp() << '\n';
    expr.GetLeftExpr().Stream( *this, indent + 1 );
    expr.GetRightExpr().Stream( *this, indent + 1 );
  }

  virtual void StreamLiteralExpr( const LiteralExpr& expr, uint32_t indent ) const override final
  {
    Indent( indent );
    *out_ << expr.GetLiteral() << '\n';
  }

  virtual void StreamParensExpr( const ParensExpr& expr, uint32_t indent ) const override final
  {
    expr.GetExpr().Stream( *this, indent );
  }

  virtual void StreamAssignExpr( const AssignExpr& expr, uint32_t indent ) const override final
  {
    expr.GetValue().Stream( *this, indent );
  }

  virtual void StreamLogicalExpr( const LogicalExpr& expr, uint32_t indent ) const override final
  {
    Indent( indent );
    *out_ << expr.GetLogicalOp() << '\n';
    expr.GetLeftExpr().Stream( *this, indent + 1 );
    expr.GetRightExpr().Stream( *this, indent + 1 );
  }

  virtual void StreamVarExpr( const VarExpr& expr, uint32_t indent ) const override final
  {
    (void)expr;
    (void)indent;
    // TODO use environment to get value stored at this variable
    // environment->get(expr.GetVariable());
  }

  virtual void StreamFuncExpr( const FuncExpr& expr, uint32_t indent ) const override final
  {
    (void)expr;
    (void)indent;
    // TODO get result of function
  }

private:
  ExprPtr root_;
  mutable std::ostream* out_ = nullptr;

};

///////////////////////////////////////////////////////////////////////////////
//
// Stream AST as text for debugging

inline std::ostream& operator<<( std::ostream& out, const AbstractSyntaxTree& ast )
{
  ast.out_ = &out; // archive for simplicity
  ast.Stream();
  return out;
}

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
