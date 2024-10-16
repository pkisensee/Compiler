///////////////////////////////////////////////////////////////////////////////
//
//  Interpreter.h
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

#include "CompilerError.h"
#include "Expr.h"
#include "Value.h"

namespace PKIsensee
{

///////////////////////////////////////////////////////////////////////////////
//
// Interprets expressions and executes statements

class Interpreter : public ExprEvaluator<Value>
{
public:

  Interpreter() = default;
  Interpreter( const Interpreter& ) = delete;
  Interpreter& operator=( const Interpreter& ) = delete;
  Interpreter( Interpreter&& ) = delete;
  Interpreter& operator=( Interpreter&& ) = delete;

  std::expected<Value, CompilerError> Evaluate( const Expr& );

private:

  Value Eval( const Expr& );

  virtual Value EvalUnaryExpr( const UnaryExpr& ) override final;
  virtual Value EvalBinaryExpr( const BinaryExpr& ) override final;
  virtual Value EvalLiteralExpr( const LiteralExpr& ) override final;
  virtual Value EvalParensExpr( const ParensExpr& ) override final;

private:

}; // class Interpreter

} // namespace PKIsensee

////////////////////////////////////////////////////////////////////////////////////////////////////
