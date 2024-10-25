///////////////////////////////////////////////////////////////////////////////
//
//  Callable.h
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
#include <functional>
#include <vector>

#include "Stmt.h"
#include "Value.h"

namespace PKIsensee
{

class Interpreter;

class Callable
{
public:
  using ArgValues = std::vector<Value>;
  using FuncType = std::function<Value( const Interpreter&, const ArgValues& )>;

  Callable() = default;

  explicit Callable( const FuncStmt* declaration ) :
    declaration_{ declaration }
  {
    assert( declaration != nullptr );
    paramCount_ = declaration->GetParams().size();
  }

  Callable( size_t paramCount, FuncType func ) :
    func_{ func },
    paramCount_{ paramCount }
  {
  }

  Callable( const Callable& ) = default;
  Callable& operator=( const Callable& ) = default;
  Callable( Callable&& ) = default;
  Callable& operator=( Callable&& ) = default;

  Value Invoke( const Interpreter&, const ArgValues& ) const;

  size_t GetParamCount() const
  {
    return paramCount_;
  }

private:

  FuncType func_;
  const FuncStmt* declaration_ = nullptr;
  size_t paramCount_ = 0u;

}; // class Callable

} // namespace PKIsensee

////////////////////////////////////////////////////////////////////////////////////////////////////
