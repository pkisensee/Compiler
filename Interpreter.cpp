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
#include "Token.h"
#include "Util.h"

using namespace PKIsensee;

#pragma warning(push)
#pragma warning(disable: 4061)

///////////////////////////////////////////////////////////////////////////////
//
// Evaluate the given expression

std::expected<Value, CompilerError> Interpreter::Evaluate( const Expr& expr ) const
{
  try
  {
    return Eval( expr );
  }
  catch( CompilerError& err )
  {
    return std::unexpected( err );
  }
}

///////////////////////////////////////////////////////////////////////////////
//
// Recursive expression evaluation

Value Interpreter::Eval( const Expr& expr ) const // private
{
  return expr.Eval( *this ); // dispatch to appropriate virtual fn
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract the value of the literal expression

Value Interpreter::EvalLiteralExpr( const LiteralExpr& literalExpr ) const // virtual
{
  return literalExpr.GetLiteral();
}

///////////////////////////////////////////////////////////////////////////////
//
// Generate the value of the unary expression

Value Interpreter::EvalUnaryExpr( const UnaryExpr& expr ) const // virtual
{
  const Value value = Eval( expr.GetExpr() );
  switch( expr.GetUnaryOp().GetType() )
  {
  case TokenType::Not:
    return Value{ !value.ToBool() };
  case TokenType::Minus:
    return value.GetNegativeValue();
  default:
    throw CompilerError( "Unexpected unary operator", expr.GetUnaryOp() );
  }
}

///////////////////////////////////////////////////////////////////////////////
//
// Generate the value of the binary expression

Value Interpreter::EvalBinaryExpr( const BinaryExpr& expr ) const // virtual
{
  const Value lhs = Eval( expr.GetLeftExpr() );
  const Value rhs = Eval( expr.GetRightExpr() );

  const Token token = expr.GetBinaryOp();
  try
  {
    switch( token.GetType() )
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
    default:
      throw CompilerError( "Unexpected binary operator", expr.GetBinaryOp() );
    }
  }
  catch( CompilerError& err )
  {
    // Deeper errors may not have token information, so ensure it is recorded
    err.SetToken( token );
    throw err;
  }
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract the value of the parenthesized expression

Value Interpreter::EvalParensExpr( const ParensExpr& parensExpr ) const // virtual
{
  return Eval( parensExpr.GetExpr() );
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract the value of the assignment expression

Value Interpreter::EvalAssignExpr( const AssignExpr& assignExpr ) const // virtual
{
  return Eval( assignExpr.GetValue() );
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract the value of the logical expression

Value Interpreter::EvalLogicalExpr( const LogicalExpr& expr ) const // virtual
{
  const Value lhs = Eval( expr.GetLeftExpr() );
  const Value rhs = Eval( expr.GetRightExpr() );

  const Token token = expr.GetLogicalOp();
  try
  {
    switch( token.GetType() )
    {
    case TokenType::And: return lhs && rhs;
    case TokenType::Or:  return lhs || rhs;
    default:
      throw CompilerError( "Unexpected logical operator", expr.GetLogicalOp() );
    }
  }
  catch( CompilerError& err )
  {
    // Deeper errors may not have token information, so ensure it is recorded
    err.SetToken( token );
    throw err;
  }
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract the value of the variable expression

Value Interpreter::EvalVarExpr( const VarExpr& ) const // virtual
{
  // TODO use current environment to get the value stored for this variable
  // return environment->get(varExpr.GetVariable());
  return {};
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract the value of the function expression

Value Interpreter::EvalFuncExpr( const FuncExpr& ) const // virtual
{
  // See Lox Interpreter.cpp
  return {};
}

#pragma warning(pop) // disable 4061

///////////////////////////////////////////////////////////////////////////////
