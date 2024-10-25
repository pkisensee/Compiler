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
#include "Environment.h"
#include "Expr.h"
#include "Stmt.h"
#include "Value.h"

namespace PKIsensee
{

///////////////////////////////////////////////////////////////////////////////
//
// Interprets expressions and executes statements

class Interpreter : public ExprEvaluator<Value>, public StmtEvaluator
{
public:

  Interpreter();

  // Disable copy/move
  Interpreter( const Interpreter& ) = delete;
  Interpreter& operator=( const Interpreter& ) = delete;
  Interpreter( Interpreter&& ) = delete;
  Interpreter& operator=( Interpreter&& ) = delete;

  std::expected<Value, CompilerError> Evaluate( const Expr& ) const;

  void Execute( const StmtList&, EnvPtr ) const;
  void Execute( const Stmt& ) const;

  EnvPtr GetGlobalsEnv() const
  {
    return globals_;
  }

private:

  Value Eval( const Expr& ) const;

  // ExprEvaluator overrides
  virtual Value EvalLiteralExpr( const LiteralExpr& ) const override final;
  virtual Value EvalUnaryExpr( const UnaryExpr& ) const override final;
  virtual Value EvalBinaryExpr( const BinaryExpr& ) const override final;
  virtual Value EvalParensExpr( const ParensExpr& ) const override final;
  virtual Value EvalAssignExpr( const AssignExpr& ) const override final;
  virtual Value EvalLogicalExpr( const LogicalExpr& ) const override final;
  virtual Value EvalVarExpr( const VarExpr& ) const override final;
  virtual Value EvalFuncExpr( const FuncExpr& ) const override final;

  virtual void EvalBlockStmt( const BlockStmt& ) override final;
  virtual void EvalExprStmt( const ExprStmt& ) override final;
  virtual void EvalIfStmt( const IfStmt& ) override final;
  virtual void EvalWhileStmt( const WhileStmt& ) override final;
  virtual void EvalReturnStmt( const ReturnStmt& ) override final;
  virtual void EvalFuncStmt( const FuncStmt& ) override final;
  virtual Value EvalVarDeclStmt( const VarDeclStmt& ) override final;
  virtual void EvalPrintStmt( const PrintStmt& ) override final;

private:

  EnvPtr globals_;
  EnvPtr environment_;

private:

  // Restore previous environment at each scope exit
  class EnvironmentGuard {
  public:
    EnvironmentGuard( Interpreter& i, EnvPtr env ) :
      interpreter_( i ),
      previousEnv_( env )
    {
    }

    ~EnvironmentGuard()
    {
      interpreter_.environment_ = previousEnv_;
    }

    // Disable copy/move
    EnvironmentGuard( const EnvironmentGuard& ) = delete;
    EnvironmentGuard& operator=( const EnvironmentGuard& ) = delete;
    EnvironmentGuard( EnvironmentGuard&& ) = delete;
    EnvironmentGuard& operator=( EnvironmentGuard&& ) = delete;


  private:
    Interpreter& interpreter_;
    EnvPtr previousEnv_;
  };

}; // class Interpreter

} // namespace PKIsensee

////////////////////////////////////////////////////////////////////////////////////////////////////
