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

#include "Expr.h"
#include "Stmt.h"
#include "Value.h"

namespace PKIsensee
{

class CompilerError;
class Environment;
using EnvPtr = std::shared_ptr<Environment>;

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

  Value Evaluate( const Expr& ) const; // TODO private?

  void Execute( const StmtList& ) const;
  void Execute( const StmtList&, EnvPtr ) const;
  void Execute( const Stmt& ) const;

  EnvPtr GetGlobalEnv() const
  {
    return globalEnv_;
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

  virtual void EvalBlockStmt( const BlockStmt& ) const override final; // TODO Eval -> Exec
  virtual void EvalExprStmt( const ExprStmt& ) const override final;
  virtual void EvalIfStmt( const IfStmt& ) const override final;
  virtual void EvalWhileStmt( const WhileStmt& ) const override final;
  virtual void EvalReturnStmt( const ReturnStmt& ) const override final;
  virtual void EvalFuncStmt( const FuncStmt& ) const override final;
  virtual void EvalVarDeclStmt( const VarDeclStmt& ) const override final;
  virtual void EvalPrintStmt( const PrintStmt& ) const override final;

private:

  EnvPtr globalEnv_;
  mutable EnvPtr environment_;

private:

  // Restore previous environment at each scope exit
  class EnvironmentGuard {
  public:
    EnvironmentGuard( const Interpreter& i, EnvPtr env ) :
      interpreter_( i ),
      previousEnv_( i.environment_ )
    {
      i.environment_ = env;
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
    const Interpreter& interpreter_;
    EnvPtr previousEnv_;
  };

}; // class Interpreter

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
