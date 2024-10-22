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

Value AssignExpr::Eval( const ExprEvaluator<Value>& exprEvaluator ) const // virtual
{
  return exprEvaluator.EvalAssignExpr( *this );
}

Value VarExpr::Eval( const ExprEvaluator<Value>& exprEvaluator ) const // virtual
{
  return exprEvaluator.EvalVarExpr( *this );
}

Value FuncExpr::Eval( const ExprEvaluator<Value>& exprEvaluator ) const // virtual
{
  return exprEvaluator.EvalFuncExpr( *this );
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

void AssignExpr::Stream( const ExprStreamer& exprStreamer, uint32_t indent ) const // virtual
{
  exprStreamer.StreamAssignExpr( *this, indent );
}

void VarExpr::Stream( const ExprStreamer& exprStreamer, uint32_t indent ) const // virtual
{
  exprStreamer.StreamVarExpr( *this, indent );
}

void FuncExpr::Stream( const ExprStreamer& exprStreamer, uint32_t indent ) const // virtual
{
  exprStreamer.StreamFuncExpr( *this, indent );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
