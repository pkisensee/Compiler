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

#include "Value.h"

namespace PKIsensee
{

class Interpreter;

class Callable
{
public:
  using ArgValues = std::vector<Value>;
  using FuncType = std::function<Value( const Interpreter&, const ArgValues& )>;

  Callable() = delete;
  Callable( uint32_t paramCount, FuncType func ) :
    paramCount_{ paramCount },
    func_{ func }
  {
  }

  // Disable copy/move
  Callable( const Callable& ) = delete;
  Callable& operator=( const Callable& ) = delete;
  Callable( Callable&& ) = delete;
  Callable& operator=( Callable&& ) = delete;

  Value Invoke( const Interpreter&, const ArgValues& ) const;

  uint32_t GetParamCount() const
  {
    return paramCount_;
  }

private:

  uint32_t paramCount_ = 0;
  FuncType func_;

}; // class Callable

} // namespace PKIsensee

////////////////////////////////////////////////////////////////////////////////////////////////////
