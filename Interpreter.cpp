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

// #include <iostream> // TODO temp for debugging
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
  globalEnv_( std::make_shared<Environment>() ),
  environment_( globalEnv_ )
{
  //globalEnv_->Define( "genre", Value{ std::string("Pop") } );
}

///////////////////////////////////////////////////////////////////////////////
//
// Execute the statements in the global environment

void Interpreter::Execute( const StmtList& statements ) const
{
  try
  {
    for( const auto& statement : statements )
      Execute( *statement );
  }
  catch( CompilerError& err )
  {
    // TODO unexpected
    std::cout << err.GetErrorMessage();
  }
}

///////////////////////////////////////////////////////////////////////////////
//
// Execute the statements in the given environment

void Interpreter::Execute( const StmtList& statements, EnvPtr env ) const
{
  EnvironmentGuard eg{ *this, env };
  for( const auto& statement : statements )
    Execute( *statement );
}

///////////////////////////////////////////////////////////////////////////////
//
// Evaluate the given expression recursively

Value Interpreter::Evaluate( const Expr& expr ) const
{
  return expr.Eval( *this ); // dispatch to virtual fn
}

///////////////////////////////////////////////////////////////////////////////
//
// Execute the given statement recursively

void Interpreter::Execute( const Stmt& stmt ) const // private
{
  stmt.Execute( *this );
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
  const Value value = Evaluate( expr.GetExpr() );
  switch( expr.GetUnaryOp().GetType() )
  {
  case TokenType::Not:
    return Value{ !value.IsTrue() };
  case TokenType::Minus:
    return -value;
  default:
    throw CompilerError( "Unexpected unary operator", expr.GetUnaryOp() );
  }
}

///////////////////////////////////////////////////////////////////////////////
//
// Generate the value of the binary expression

Value Interpreter::EvalBinaryExpr( const BinaryExpr& expr ) const // virtual
{
  const Value lhs = Evaluate( expr.GetLeftExpr() );
  const Value rhs = Evaluate( expr.GetRightExpr() );

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
    case TokenType::Modulus:          return lhs % rhs;
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
  return Evaluate( parensExpr.GetExpr() );
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract the value of the assignment expression

Value Interpreter::EvalAssignExpr( const AssignExpr& assignExpr ) const // virtual
{
  Value newValue = Evaluate( assignExpr.GetValue() );
  environment_->Assign( assignExpr.GetVariable(), newValue );
  return newValue;
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract the value of the logical expression

Value Interpreter::EvalLogicalExpr( const LogicalExpr& logicalExpr ) const // virtual
{
  const Token logicalOp = logicalExpr.GetLogicalOp();
  try
  {
    const Value lhs = Evaluate( logicalExpr.GetLeftExpr() );
    switch( logicalOp.GetType() )
    {
    case TokenType::And:
    {
      const Value rhs = Evaluate( logicalExpr.GetRightExpr() );
      return lhs && rhs;
    }
    case TokenType::Or:
    {
      if( lhs.IsTrue() ) // short circuit
        return Value{ true };
      const Value rhs = Evaluate( logicalExpr.GetRightExpr() );
      return lhs || rhs;
    }
    default:
      throw CompilerError( "Unexpected logical operator", logicalExpr.GetLogicalOp() );
    }
  }
  catch( CompilerError& err )
  {
    // Deeper errors may not have token information, so ensure it is recorded
    err.SetToken( logicalOp );
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
    argValues.push_back( Evaluate( *arg ) );

  Value callee = Evaluate( funcExpr.GetFunc() );
  if( callee.GetType() != ValueType::Func )
    throw CompilerError( "Can only call functions" );

  /*
  std::cout << "Call " << funcExpr.GetFuncName().GetValue() << '(';
  bool first = true;
  for( const Value& arg : argValues )
  {
    if( !first )
      std::cout << ", ";
    std::cout << arg.ToString();
    first = false;
  }
  std::cout << ")\n";
  */

  Callable callable = callee.GetFunc();
  return callable.Invoke( *this, argValues );
}

///////////////////////////////////////////////////////////////////////////////
//
// Evaluate the block statement

void Interpreter::ExecBlockStmt( const BlockStmt& stmt ) const // virtual
{
  EnvPtr newEnvironment = std::make_shared<Environment>( environment_ );
  Execute( stmt.GetStatements(), newEnvironment );
}

///////////////////////////////////////////////////////////////////////////////
//
// Evaluate the expression statement

void Interpreter::ExecExprStmt( const ExprStmt& exprStmt ) const // virtual
{
  Evaluate( exprStmt.GetExpr() );
}

///////////////////////////////////////////////////////////////////////////////
//
// Evaluate the if statement

void Interpreter::ExecIfStmt( const IfStmt& ifStmt ) const // virtual
{
  if( Evaluate( ifStmt.GetCondition() ).IsTrue() )
    Execute( ifStmt.GetBranchTrue() );
  else if( ifStmt.HasElseBranch() )
    Execute( ifStmt.GetBranchFalse() );
}

///////////////////////////////////////////////////////////////////////////////
//
// Evaluate the while statement

void Interpreter::ExecWhileStmt( const WhileStmt& whileStmt ) const // virtual
{
  while( Evaluate( whileStmt.GetCondition() ).IsTrue() )
    Execute( whileStmt.GetBody() );
}

///////////////////////////////////////////////////////////////////////////////
//
// Evaluate the return statement

void Interpreter::ExecReturnStmt( const ReturnStmt& returnStmt ) const // virtual
{
  Value value;
  if( returnStmt.HasValue() )
    value = Evaluate( returnStmt.GetValue() );

  // Return statements can happen at any level within a function, so the simplest
  // way to unwind is to use exception handling. Caught in Callable::Invoke.
  throw ReturnException( value );
}

///////////////////////////////////////////////////////////////////////////////
//
// Evaluate the function

void Interpreter::ExecFuncStmt( const FuncStmt& funcStmt ) const // virtual
{
  Value callable{ Callable( &funcStmt ) };
  environment_->Define( funcStmt.GetName().GetValue(), callable );
}

///////////////////////////////////////////////////////////////////////////////
//
// Evaluate the variable declaration

void Interpreter::ExecVarDeclStmt( const VarDeclStmt& varDeclStmt ) const // virtual
{
  Value value;
  if( varDeclStmt.HasInitializer() )
    value = Evaluate( varDeclStmt.GetInitializer() );
  environment_->Define( varDeclStmt.GetName().GetValue(), value );
}

///////////////////////////////////////////////////////////////////////////////
//
// Evaluate the print statement

void Interpreter::ExecPrintStmt( const PrintStmt& printStmt ) const // virtual
{
  Value value = Evaluate( printStmt.GetExpr() );
  std::print( "{}\n", value.ToString() );
}

#pragma warning(pop) // disable 4061

///////////////////////////////////////////////////////////////////////////////
