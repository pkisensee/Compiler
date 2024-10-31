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
  stmtEvaluator.ExecBlockStmt( *this );
}

void ExprStmt::Execute( const StmtEvaluator& stmtEvaluator ) const // virtual
{
  stmtEvaluator.ExecExprStmt( *this );
}

void IfStmt::Execute( const StmtEvaluator& stmtEvaluator ) const // virtual
{
  stmtEvaluator.ExecIfStmt( *this );
}

void WhileStmt::Execute( const StmtEvaluator& stmtEvaluator ) const // virtual
{
  stmtEvaluator.ExecWhileStmt( *this );
}

void ReturnStmt::Execute( const StmtEvaluator& stmtEvaluator ) const // virtual
{
  stmtEvaluator.ExecReturnStmt( *this );
}

void FuncStmt::Execute( const StmtEvaluator& stmtEvaluator ) const // virtual
{
  stmtEvaluator.ExecFuncStmt( *this );
}

void VarDeclStmt::Execute( const StmtEvaluator& stmtEvaluator ) const // virtual
{
  stmtEvaluator.ExecVarDeclStmt( *this );
}

void PrintStmt::Execute( const StmtEvaluator& stmtEvaluator ) const // virtual
{
  stmtEvaluator.ExecPrintStmt( *this );
}

///////////////////////////////////////////////////////////////////////////////
