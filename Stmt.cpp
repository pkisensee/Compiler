///////////////////////////////////////////////////////////////////////////////
//
//  Stmt.cpp
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

#include "Stmt.h"

using namespace PKIsensee;

void BlockStmt::Execute( const StmtEvaluator& stmtEvaluator ) const // virtual
{
  stmtEvaluator.EvalBlockStmt( *this );
}

void ExprStmt::Execute( const StmtEvaluator& stmtEvaluator ) const // virtual
{
  stmtEvaluator.EvalExprStmt( *this );
}

void IfStmt::Execute( const StmtEvaluator& stmtEvaluator ) const // virtual
{
  stmtEvaluator.EvalIfStmt( *this );
}

void WhileStmt::Execute( const StmtEvaluator& stmtEvaluator ) const // virtual
{
  stmtEvaluator.EvalWhileStmt( *this );
}

void ReturnStmt::Execute( const StmtEvaluator& stmtEvaluator ) const // virtual
{
  stmtEvaluator.EvalReturnStmt( *this );
}

void FuncStmt::Execute( const StmtEvaluator& stmtEvaluator ) const // virtual
{
  stmtEvaluator.EvalFuncStmt( *this );
}

void VarDeclStmt::Execute( const StmtEvaluator& stmtEvaluator ) const // virtual
{
  stmtEvaluator.EvalVarDeclStmt( *this );
}

void PrintStmt::Execute( const StmtEvaluator& stmtEvaluator ) const // virtual
{
  stmtEvaluator.EvalPrintStmt( *this );
}

///////////////////////////////////////////////////////////////////////////////
