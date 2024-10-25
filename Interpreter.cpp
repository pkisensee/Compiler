///////////////////////////////////////////////////////////////////////////////
//
//  Interpreter.cpp
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

#include <print>

#include "Callable.h"
#include "CompilerError.h"
#include "Environment.h"
#include "Interpreter.h"
#include "Token.h"
#include "Util.h"

using namespace PKIsensee;

#pragma warning(push)
#pragma warning(disable: 4061)

///////////////////////////////////////////////////////////////////////////////
//
// Construct the interpreter and global environment

Interpreter::Interpreter() :
  globals_( std::make_shared<Environment>() ),
  environment_( globals_ )
{
}

///////////////////////////////////////////////////////////////////////////////
//
// Evaluate the given expression

// TODO return value may move to a higher level
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

void Interpreter::Execute( const StmtList& statements, EnvPtr env ) const
{
  // TODO add try/catch
  EnvironmentGuard eg( *this, env );
  for( const auto& statement : statements )
    Execute( *statement );
}

void Interpreter::Execute( const Stmt& stmt ) const // TODO private
{
  stmt.Execute( *this );
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
    return Value{ !value.IsTrue() };
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
  // TODO incorporate environment
  return Eval( assignExpr.GetValue() );
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract the value of the logical expression

Value Interpreter::EvalLogicalExpr( const LogicalExpr& logicalExpr ) const // virtual
{
  const Value lhs = Eval( logicalExpr.GetLeftExpr() );
  const Value rhs = Eval( logicalExpr.GetRightExpr() );

  const Token token = logicalExpr.GetLogicalOp();
  try
  {
    switch( token.GetType() )
    {
    case TokenType::And: return lhs && rhs;
    case TokenType::Or:  return lhs || rhs;
    default:
      throw CompilerError( "Unexpected logical operator", logicalExpr.GetLogicalOp() );
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

Value Interpreter::EvalVarExpr( const VarExpr& varExpr ) const // virtual
{
  return environment_->GetValue( varExpr.GetVariable() );
}

///////////////////////////////////////////////////////////////////////////////
//
// Invoke the function

Value Interpreter::EvalFuncExpr( const FuncExpr& funcExpr ) const // virtual
{
  // Evaluate function arguments
  std::vector<Value> argValues;
  for( const auto& arg : funcExpr.GetArgs() )
    argValues.push_back( Eval( *arg ) );

  Value callee = Eval( funcExpr.GetFuncName() );
  if( callee.GetType() != ValueType::Func )
    throw CompilerError( "Can only call functions" );

  Callable callable = callee.GetFunc();
  return callable.Invoke( *this, argValues );
}

///////////////////////////////////////////////////////////////////////////////
//
// Evaluate the block statement

void Interpreter::EvalBlockStmt( const BlockStmt& stmt ) const // virtual
{
  EnvironmentGuard eg{ *this, environment_ };
  for( const auto& statement : stmt.GetStatements() )
    Execute( *statement );
}

///////////////////////////////////////////////////////////////////////////////
//
// Evaluate the expression statement

void Interpreter::EvalExprStmt( const ExprStmt& exprStmt ) const // virtual
{
  Eval( exprStmt.GetExpr() );
}

///////////////////////////////////////////////////////////////////////////////
//
// Evaluate the if statement

void Interpreter::EvalIfStmt( const IfStmt& ifStmt ) const // virtual
{
  if( Eval( ifStmt.GetCondition() ).IsTrue() )
    Execute( ifStmt.GetBranchTrue() );
  else if( ifStmt.HasElseBranch() )
    Execute( ifStmt.GetBranchFalse() );
}

///////////////////////////////////////////////////////////////////////////////
//
// Evaluate the while statement

void Interpreter::EvalWhileStmt( const WhileStmt& whileStmt ) const // virtual
{
  while( Eval( whileStmt.GetCondition() ).IsTrue() )
    Execute( whileStmt.GetBody() );
}

///////////////////////////////////////////////////////////////////////////////
//
// Evaluate the return statement

void Interpreter::EvalReturnStmt( const ReturnStmt& returnStmt ) const // virtual
{
  Value value;
  if( returnStmt.HasValue() )
    value = Eval( returnStmt.GetValue() );

  // A return statement can happen at any level within a function, so the
  // easiest way to quickly unwind is to use exception handling. Caught in
  // TODO
  throw ReturnException( value );
}

///////////////////////////////////////////////////////////////////////////////
//
// Evaluate the function

void Interpreter::EvalFuncStmt( const FuncStmt& funcStmt ) const // virtual
{
  Value val{ Callable( &funcStmt ) };
  environment_->Define( funcStmt.GetName().GetValue(), val );
}

///////////////////////////////////////////////////////////////////////////////
//
// Evaluate the variable declaration

void Interpreter::EvalVarDeclStmt( const VarDeclStmt& varDeclStmt ) const // virtual
{
  Value value;
  if( varDeclStmt.HasInitializer() )
    value = Eval( varDeclStmt.GetInitializer() );
  environment_->Define( varDeclStmt.GetName().GetValue(), value );
}

///////////////////////////////////////////////////////////////////////////////
//
// Evaluate the print statement

void Interpreter::EvalPrintStmt( const PrintStmt& printStmt ) const // virtual
{
  std::print( "{}\n", Eval( printStmt.GetExpr() ).ToString() );
}

#pragma warning(pop) // disable 4061

///////////////////////////////////////////////////////////////////////////////
