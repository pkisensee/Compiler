///////////////////////////////////////////////////////////////////////////////
//
//  Interpreter.cpp
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

#include "CompilerError.h"
#include "Interpreter.h"

using namespace PKIsensee;

#pragma warning(push)
#pragma warning(disable: 4061)

///////////////////////////////////////////////////////////////////////////////
//
// Generate the value of the unary expression

Value Interpreter::VisitUnaryExpr( const UnaryExpr& expr ) // virtual TODO EvalUnaryExpr()?
{
  const Value value = Evaluate( expr.GetExpr() );
  switch( expr.GetUnaryOp().GetType() )
  {
  case TokenType::Not:
    return Value{ !value.IsTrueEquivalent() };
  case TokenType::Minus:
    return value.GetNegativeValue();
  default:
    throw CompilerError( expr.GetUnaryOp(), "Unexpected unary token" );
  }
}
#pragma warning(pop)

///////////////////////////////////////////////////////////////////////////////
//
// Generate the value of the binary expression

Value Interpreter::VisitBinaryExpr( const BinaryExpr& expr ) // virtual
{
  const Value lhs = Evaluate( expr.GetLeftExpr() );
  const Value rhs = Evaluate( expr.GetRightExpr() );

  switch( expr.GetBinaryOp().GetType() )
  {
  case TokenType::IsEqual:          return Value{ lhs == rhs };
  case TokenType::NotEqual:         return Value{ lhs != rhs };
  case TokenType::LessThan:         return Value{ lhs < rhs };
  case TokenType::GreaterThan:      return Value{ lhs > rhs };
  case TokenType::LessThanEqual:    return Value{ lhs <= rhs };
  case TokenType::GreaterThanEqual: return Value{ lhs >= rhs };
  case TokenType::Plus:             return lhs + rhs;
  case TokenType::Minus:            return lhs - rhs;
  case TokenType::Multiply:         return lhs * rhs;
  case TokenType::Divide:           return lhs / rhs;
  }

  return {};
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract the value of the literal expression

Value Interpreter::VisitLiteralExpr( const LiteralExpr& ) // virtual
{
  return {};
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract the value of the parenthesized expression

Value Interpreter::VisitParensExpr( const ParensExpr& ) // virtual
{
  return {};
}



///////////////////////////////////////////////////////////////////////////////
