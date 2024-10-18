///////////////////////////////////////////////////////////////////////////////
//
//  Stmt.h
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
#include <memory>
#include <vector>

namespace PKIsensee
{

class Stmt;
using StmtPtr = std::unique_ptr<Stmt>;
using StmtList = std::vector<StmtPtr>;

///////////////////////////////////////////////////////////////////////////////
//
// Statement abstract base class interface

class Stmt
{
public:
  Stmt() = default;
  virtual ~Stmt() = default;

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

private:
  ExprPtr value_;

}; // class ReturnStmt

///////////////////////////////////////////////////////////////////////////////
//
// Function Statement

class FuncStmt : public Stmt
{
public:
  FuncStmt() = delete;
  FuncStmt( Token fnName, TokenList params, StmtList body ) :
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

private:
  Token fnName_;
  TokenList params_;
  StmtList body_;

}; // class FuncStmt

///////////////////////////////////////////////////////////////////////////////
//
// Print Statement

class PrintStmt : public Stmt
{
public:
  PrintStmt() = delete;
  explicit PrintStmt( ExprPtr value ) :
    value_{ std::move( value ) }
  {
  }

  // Disable copies, allow moves
  PrintStmt( const PrintStmt& ) = delete;
  PrintStmt& operator=( const PrintStmt& ) = delete;
  PrintStmt( PrintStmt&& ) = default;
  PrintStmt& operator=( PrintStmt&& ) = default;

private:
  ExprPtr value_;

}; // class PrintStmt

} // namespace PKIsensee

////////////////////////////////////////////////////////////////////////////////////////////////////
