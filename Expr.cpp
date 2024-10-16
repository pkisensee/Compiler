///////////////////////////////////////////////////////////////////////////////
//
//  Expr.cpp
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

#include "Expr.h"

using namespace PKIsensee;

///////////////////////////////////////////////////////////////////////////////
//
// Expression evaluators using visitor pattern

Value UnaryExpr::Eval( ExprEvaluator<Value>& exprEvaluator ) const // virtual
{
  return exprEvaluator.EvalUnaryExpr( *this );
}

Value BinaryExpr::Eval( ExprEvaluator<Value>& exprEvaluator ) const // virtual
{
  return exprEvaluator.EvalBinaryExpr( *this );
}

Value LiteralExpr::Eval( ExprEvaluator<Value>& exprEvaluator ) const // virtual
{
  return exprEvaluator.EvalLiteralExpr( *this );
}

Value ParensExpr::Eval( ExprEvaluator<Value>& exprEvaluator ) const // virtual
{
  return exprEvaluator.EvalParensExpr( *this );
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

void ParensExpr::Stream( std::ostream& out, uint32_t indent ) const // virtual
{
  expr_->Stream( out, indent );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
