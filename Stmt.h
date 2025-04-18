///////////////////////////////////////////////////////////////////////////////
//
//  Stmt.h
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

#pragma once
#include <memory>
#include <vector>

#include "Expr.h"
#include "Token.h"
#include "Value.h"

namespace PKIsensee
{

class Stmt;
using StmtPtr = std::unique_ptr<Stmt>; // TODO consider shared_ptr to avoid unreadable std::moves
using StmtList = std::vector<StmtPtr>;
using Param = std::pair<Token, Token>; // type and name
using ParamList = std::vector<Param>;

class BlockStmt;
class ExprStmt;
class IfStmt;
class WhileStmt;
class ReturnStmt;
class FuncStmt;
class VarDeclStmt;
class PrintStmt;

///////////////////////////////////////////////////////////////////////////////
//
// Visitor interface used to walk statement list to execute the list.
// Designed as a mix-in base class.
// TODO change to StmtExecutor

class StmtEvaluator
{
public:
  virtual ~StmtEvaluator() = default;
  StmtEvaluator() = default;
  StmtEvaluator( const StmtEvaluator& ) = default;
  StmtEvaluator& operator=( const StmtEvaluator& ) = default;
  StmtEvaluator( StmtEvaluator&& ) = default;
  StmtEvaluator& operator=( StmtEvaluator&& ) = default;

  virtual void ExecBlockStmt( const BlockStmt& ) const = 0;
  virtual void ExecExprStmt( const ExprStmt& ) const = 0;
  virtual void ExecIfStmt( const IfStmt& ) const = 0;
  virtual void ExecWhileStmt( const WhileStmt& ) const = 0;
  virtual void ExecReturnStmt( const ReturnStmt& ) const = 0;
  virtual void ExecFuncStmt( const FuncStmt& ) const = 0;
  virtual void ExecVarDeclStmt( const VarDeclStmt& ) const = 0;
  virtual void ExecPrintStmt( const PrintStmt& ) const = 0;

};

///////////////////////////////////////////////////////////////////////////////
//
// Statement abstract base class interface

class Stmt
{
public:
  Stmt() = default;
  virtual ~Stmt() = default;

  virtual void Execute( const StmtEvaluator& ) const = 0;

}; // class Stmt

///////////////////////////////////////////////////////////////////////////////
//
// Block Statement

class BlockStmt : public Stmt
{
public:
  BlockStmt() = delete;
  explicit BlockStmt( StmtList statements ) :
    statements_( std::move(statements) )
  {
  }

  // Disable copies, allow moves
  BlockStmt( const BlockStmt& ) = delete;
  BlockStmt& operator=( const BlockStmt& ) = delete;
  BlockStmt( BlockStmt&& ) = default;
  BlockStmt& operator=( BlockStmt&& ) = default;

  const StmtList& GetStatements() const
  {
    return statements_;
  }

  virtual void Execute( const StmtEvaluator& ) const override final;

private:
  StmtList statements_;

}; // class BlockStmt

///////////////////////////////////////////////////////////////////////////////
//
// Expression Statement

class ExprStmt : public Stmt
{
public:
  ExprStmt() = delete;
  explicit ExprStmt( ExprPtr expr ) :
    expr_( std::move( expr ) )
  {
  }

  // Disable copies, allow moves
  ExprStmt( const ExprStmt& ) = delete;
  ExprStmt& operator=( const ExprStmt& ) = delete;
  ExprStmt( ExprStmt&& ) = default;
  ExprStmt& operator=( ExprStmt&& ) = default;

  const Expr& GetExpr() const
  {
    return *expr_;
  }

  virtual void Execute( const StmtEvaluator& ) const override final;

private:
  ExprPtr expr_;

}; // class ExprStmt

///////////////////////////////////////////////////////////////////////////////
//
// If Statement

class IfStmt : public Stmt
{
public:
  IfStmt() = delete;
  IfStmt( ExprPtr condition, StmtPtr branchTrue, StmtPtr branchFalse ) :
    condition_( std::move( condition ) ),
    branchTrue_( std::move( branchTrue ) ),
    branchFalse_( std::move( branchFalse ) )
  {
  }

  // Disable copies, allow moves
  IfStmt( const IfStmt& ) = delete;
  IfStmt& operator=( const IfStmt& ) = delete;
  IfStmt( IfStmt&& ) = default;
  IfStmt& operator=( IfStmt&& ) = default;

  const Expr& GetCondition() const
  {
    return *condition_;
  }

  bool HasElseBranch() const
  {
    return branchFalse_ != nullptr;
  }

  const Stmt& GetBranchTrue() const
  {
    return *branchTrue_;
  }

  const Stmt& GetBranchFalse() const
  {
    return *branchFalse_;
  }

  virtual void Execute( const StmtEvaluator& ) const override final;

private:
  ExprPtr condition_;
  StmtPtr branchTrue_;
  StmtPtr branchFalse_;

}; // class IfStmt

///////////////////////////////////////////////////////////////////////////////
//
// While Statement

class WhileStmt : public Stmt
{
public:
  WhileStmt() = delete;
  WhileStmt( ExprPtr condition, StmtPtr body ) :
    condition_( std::move( condition ) ),
    body_( std::move( body ) )
  {
  }

  // Disable copies, allow moves
  WhileStmt( const WhileStmt& ) = delete;
  WhileStmt& operator=( const WhileStmt& ) = delete;
  WhileStmt( WhileStmt&& ) = default;
  WhileStmt& operator=( WhileStmt&& ) = default;

  const Expr& GetCondition() const
  {
    return *condition_;
  }

  const Stmt& GetBody() const
  {
    return *body_;
  }

  virtual void Execute( const StmtEvaluator& ) const override final;

private:
  ExprPtr condition_;
  StmtPtr body_;

}; // class WhileStmt

///////////////////////////////////////////////////////////////////////////////
//
// Return Statement

class ReturnStmt : public Stmt
{
public:
  ReturnStmt() = delete;
  explicit ReturnStmt( ExprPtr value ) :
    value_{ std::move(value) }
  {
  }

  // Disable copies, allow moves
  ReturnStmt( const ReturnStmt& ) = delete;
  ReturnStmt& operator=( const ReturnStmt& ) = delete;
  ReturnStmt( ReturnStmt&& ) = default;
  ReturnStmt& operator=( ReturnStmt&& ) = default;

  bool HasValue() const
  {
    return value_.get() != nullptr;
  }

  const Expr& GetValue() const
  {
    return *value_;
  }

  virtual void Execute( const StmtEvaluator& ) const override final;

private:
  ExprPtr value_;

}; // class ReturnStmt

///////////////////////////////////////////////////////////////////////////////
//
// Return Value
//
// A return statement can happen at any level within a function, so the
// easiest way to quickly unwind is to use exception handling

class ReturnException : public std::exception
{
public:
  ReturnException() = delete;
  explicit ReturnException( const Value& value ) :
    std::exception{}, 
    value_{ value }
  {
  }

  ReturnException( const ReturnException& ) = default;
  ReturnException& operator=( const ReturnException& ) = default;
  ReturnException( ReturnException&& ) = default;
  ReturnException& operator=( ReturnException&& ) = default;

  const Value& GetValue() const
  {
    return value_;
  }

private:
  Value value_;

};

///////////////////////////////////////////////////////////////////////////////
//
// Function Statement

class FuncStmt : public Stmt
{
public:
  FuncStmt() = delete;
  FuncStmt( Token fnName, ParamList params, StmtList body ) :
    fnName_( fnName ),
    params_( std::move(params) ),
    body_( std::move(body) )
  {
  }

  // Disable copies, allow moves
  FuncStmt( const FuncStmt& ) = delete;
  FuncStmt& operator=( const FuncStmt& ) = delete;
  FuncStmt( FuncStmt&& ) = default;
  FuncStmt& operator=( FuncStmt&& ) = default;

  Token GetName() const
  {
    return fnName_;
  }

  const ParamList& GetParams() const
  {
    return params_;
  }

  const StmtList& GetBody() const
  {
    return body_;
  }

  virtual void Execute( const StmtEvaluator& ) const override final;

private:
  Token fnName_;
  ParamList params_;
  StmtList body_;

}; // class FuncStmt

///////////////////////////////////////////////////////////////////////////////
//
// Variable Declaration Statement

class VarDeclStmt : public Stmt
{
public:
  VarDeclStmt() = delete;
  VarDeclStmt( Token varType, Token varName, ExprPtr initializer ) :
    varType_{ varType },
    varName_{ varName },
    initializer_{ std::move( initializer ) }
  {
  }

  // Disable copies, allow moves
  VarDeclStmt( const VarDeclStmt& ) = delete;
  VarDeclStmt& operator=( const VarDeclStmt& ) = delete;
  VarDeclStmt( VarDeclStmt&& ) = default;
  VarDeclStmt& operator=( VarDeclStmt&& ) = default;

  Token GetName() const
  {
    return varName_;
  }

  bool HasInitializer() const
  {
    return initializer_ != nullptr;
  }

  const Expr& GetInitializer() const
  {
    return *initializer_;
  }

  virtual void Execute( const StmtEvaluator& ) const override final;

private:
  Token varType_; // TODO need?
  Token varName_;
  ExprPtr initializer_;

}; // class VarDeclStmt

///////////////////////////////////////////////////////////////////////////////
//
// Print Statement

class PrintStmt : public Stmt
{
public:
  PrintStmt() = delete;
  explicit PrintStmt( ExprPtr expr ) :
    expr_{ std::move( expr ) }
  {
  }

  // Disable copies, allow moves
  PrintStmt( const PrintStmt& ) = delete;
  PrintStmt& operator=( const PrintStmt& ) = delete;
  PrintStmt( PrintStmt&& ) = default;
  PrintStmt& operator=( PrintStmt&& ) = default;

  const Expr& GetExpr() const
  {
    return *expr_;
  }

  virtual void Execute( const StmtEvaluator& ) const override final;

private:
  ExprPtr expr_;

}; // class PrintStmt

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
