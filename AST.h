///////////////////////////////////////////////////////////////////////////////
//
//  AST.h
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

#include <iostream>
#include "Expr.h"

namespace PKIsensee
{

///////////////////////////////////////////////////////////////////////////////
//
// Representation of source code
// 
// Given the expression: 1 + 2 * 3
//
// The resulting AST is:
//    +
//   / \
//  1   *
//     / \
//    2   3
//
// operator<<() expresses the AST in sideways form, where each 
// indentation is a new level in the tree
//
// + [Plus]
//   1 [Number]
//   * [Multiply]
//     2 [Number]
//     3 [Number]

class AbstractSyntaxTree
{
public:
  AbstractSyntaxTree() = delete;

  explicit AbstractSyntaxTree( ExprPtr expr ) :
    root_( std::move(expr) )
  {
  }

  // Disable copies, allow moves
  AbstractSyntaxTree( const AbstractSyntaxTree& ) = delete;
  AbstractSyntaxTree& operator=( const AbstractSyntaxTree& ) = delete;
  AbstractSyntaxTree( AbstractSyntaxTree&& ) = default;
  AbstractSyntaxTree& operator=( AbstractSyntaxTree&& ) = default;

  const Expr& GetExpr() const
  {
    return *root_;
  }

  friend std::ostream& operator<<( std::ostream&, const AbstractSyntaxTree& );

private:
  ExprPtr root_;

};

///////////////////////////////////////////////////////////////////////////////
//
// Stream AST as text for debugging

inline std::ostream& operator<<( std::ostream& out, const AbstractSyntaxTree& ast )
{
  if( ast.root_ )
    ast.root_->Stream( out, 0 );
  return out;
}

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
