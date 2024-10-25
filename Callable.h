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

namespace PKIsensee
{

class FuncStmt;
class Interpreter;
class Value;

class Callable
{
public:
  using ArgValues = std::vector<Value>;
  using FuncType = std::function<Value( const Interpreter&, const ArgValues& )>;

  Callable() = default;

  explicit Callable( const FuncStmt* );

  Callable( FuncType func, size_t paramCount ) :
    func_{ func },
    paramCount_{ paramCount }
  {
  }

  // Enable copy/move
  Callable( const Callable& ) = default;
  Callable& operator=( const Callable& ) = default;
  Callable( Callable&& ) = default;
  Callable& operator=( Callable&& ) = default;

  size_t GetParamCount() const
  {
    return paramCount_;
  }

  Value Invoke( const Interpreter&, const ArgValues& ) const;

  std::strong_ordering operator<=>( const Callable& ) const;
  bool operator==( const Callable& ) const;

private:

  FuncType func_;
  const FuncStmt* declaration_ = nullptr;
  size_t paramCount_ = 0u;

}; // class Callable

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
