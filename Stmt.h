/////////////////////////////////////////////////////////
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
  virtual ~Stmt() = default;
  Stmt() = default;

  // Disable copies and movess
  Stmt( const Stmt& ) = delete;
  Stmt& operator=( const Stmt& ) = delete;
  Stmt( Stmt&& ) = delete;
  Stmt& operator=( Stmt&& ) = delete;

}; // class Stmt

///////////////////////////////////////////////////////////////////////////////
//
// Block Statement

class BlockStmt : public Stmt
{
public:
  BlockStmt() = delete;
  explicit BlockStmt( const StmtList& );

  // Disable copy and move
  BlockStmt( const BlockStmt& ) = delete;
  BlockStmt& operator=( const BlockStmt& ) = delete;
  BlockStmt( BlockStmt&& ) = delete;
  BlockStmt& operator=( BlockStmt&& ) = delete;

private:
  StmtList statements_;

}; // class BlockStmt

///////////////////////////////////////////////////////////////////////////////
//
// Expression Statement

class ExpressionStmt : public Stmt
{
public:
  ExpressionStmt() = delete;
  explicit ExpressionStmt( ExprPtr );

  // Disable copy and move
  ExpressionStmt( const ExpressionStmt& ) = delete;
  ExpressionStmt& operator=( const ExpressionStmt& ) = delete;
  ExpressionStmt( ExpressionStmt&& ) = delete;
  ExpressionStmt& operator=( ExpressionStmt&& ) = delete;

private:
  ExprPtr expr_;

}; // class ExpressionStmt

///////////////////////////////////////////////////////////////////////////////
//
// If Statement

class IfStmt : public Stmt
{
public:
  IfStmt() = delete;
  IfStmt( ExprPtr condition, StmtPtr branchTrue, StmtPtr branchFalse );

  // Disable copy and move
  IfStmt( const IfStmt& ) = delete;
  IfStmt& operator=( const IfStmt& ) = delete;
  IfStmt( IfStmt&& ) = delete;
  IfStmt& operator=( IfStmt&& ) = delete;

private:
  ExprPtr expr_;
  StmtPtr branchTrue_;
  StmtPtr branchFalse_;

}; // class IfStmt

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

  // Disable copy and move
  ReturnStmt( const ReturnStmt& ) = delete;
  ReturnStmt& operator=( const ReturnStmt& ) = delete;
  ReturnStmt( ReturnStmt&& ) = delete;
  ReturnStmt& operator=( ReturnStmt&& ) = delete;

private:
  ExprPtr value_;

}; // class ReturnStmt

} // namespace PKIsensee

////////////////////////////////////////////////////////////////////////////////////////////////////
