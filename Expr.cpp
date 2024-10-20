///////////////////////////////////////////////////////////////////////////////
//
//  Expr.cpp
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
///////////////////////////////////////////////////////////////////////////////

#include "Expr.h"

using namespace PKIsensee;

///////////////////////////////////////////////////////////////////////////////
//
// Expression evaluators using visitor pattern

Value UnaryExpr::Eval( const ExprEvaluator<Value>& exprEvaluator ) const // virtual
{
  return exprEvaluator.EvalUnaryExpr( *this );
}

Value BinaryExpr::Eval( const ExprEvaluator<Value>& exprEvaluator ) const // virtual
{
  return exprEvaluator.EvalBinaryExpr( *this );
}

Value LiteralExpr::Eval( const ExprEvaluator<Value>& exprEvaluator ) const // virtual
{
  return exprEvaluator.EvalLiteralExpr( *this );
}

Value ParensExpr::Eval( const ExprEvaluator<Value>& exprEvaluator ) const // virtual
{
  return exprEvaluator.EvalParensExpr( *this );
}

///////////////////////////////////////////////////////////////////////////////
//
// Expression streamers using visitor pattern

void UnaryExpr::Stream( const ExprStreamer& exprStreamer, uint32_t indent ) const // virtual
{
  exprStreamer.StreamUnaryExpr( *this, indent );
}

void BinaryExpr::Stream( const ExprStreamer& exprStreamer, uint32_t indent ) const // virtual
{
  exprStreamer.StreamBinaryExpr( *this, indent );
}

void LiteralExpr::Stream( const ExprStreamer& exprStreamer, uint32_t indent ) const // virtual
{
  exprStreamer.StreamLiteralExpr( *this, indent );
}

void ParensExpr::Stream( const ExprStreamer& exprStreamer, uint32_t indent ) const // virtual
{
  exprStreamer.StreamParensExpr( *this, indent );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
