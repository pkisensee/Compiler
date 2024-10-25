///////////////////////////////////////////////////////////////////////////////
//
//  Environment.h
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
#include <cassert>
#include <memory>
#include <string_view>
#include <unordered_map>

namespace PKIsensee
{

class Callable;
class Environment;
class Token;
class Value;

using EnvPtr = std::shared_ptr<Environment>;

class Environment
{
public:
  Environment() = default;
  explicit Environment( EnvPtr enclosing ) :
    enclosingEnv_( enclosing )
  {
    assert( enclosing != nullptr );
  }

  // Disable copy/move
  Environment( const Environment& ) = delete;
  Environment& operator=( const Environment& ) = delete;
  Environment( Environment&& ) = delete;
  Environment& operator=( Environment&& ) = delete;

  void Define( std::string_view name, const Value& value )
  {
    values_.emplace( name, value );
  }

  void Define( std::string_view name, const Callable& func )
  {
    functions_.emplace( name, func );
  }

  void Assign( Token, const Value& );
  Value GetValue( Token ) const;

private:

  EnvPtr enclosingEnv_;
  std::unordered_map<std::string_view, Value> values_;
  std::unordered_map<std::string_view, Callable> functions_;

}; // class Environment

} // namespace PKIsensee

////////////////////////////////////////////////////////////////////////////////////////////////////
