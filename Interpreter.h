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

#include "Expr.h"
#include "Value.h"

namespace PKIsensee
{

///////////////////////////////////////////////////////////////////////////////
//
// Interprets expressions and executes statements

class Interpreter : public ExprVisitor<Value>
{
public:

  Value Evaluate( const Expr& expr )
  {
    return expr.Visit( *this ); // dispatch to appropriate virtual fn TODO DispatchVisit
  }

private:

  virtual Value VisitUnaryExpr( const UnaryExpr& ) override final;
  virtual Value VisitBinaryExpr( const BinaryExpr& ) override final;
  virtual Value VisitLiteralExpr( const LiteralExpr& ) override final;
  virtual Value VisitParensExpr( const ParensExpr& ) override final;

private:

}; // class Interpreter

} // namespace PKIsensee

////////////////////////////////////////////////////////////////////////////////////////////////////
