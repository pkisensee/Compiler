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

using EnvPtr = std::unique_ptr<Environment>;

class Environment
{
public:
  Environment() {};
  explicit Environment( Environment* enclosing ) :
    enclosingEnv_( enclosing )
  {
    assert( enclosing != nullptr );
  }

  // Enable copy/move
  Environment( const Environment& ) = default;
  Environment& operator=( const Environment& ) = default;
  Environment( Environment&& ) = default;
  Environment& operator=( Environment&& ) = default;

  void Define( std::string_view name, const Value& value )
  {
    values_.emplace( name, value );
  }

  void Assign( Token, const Value& );
  Value GetValue( Token ) const;

private:

  Environment* enclosingEnv_ = nullptr;
  std::unordered_map<std::string_view, Value> values_;

}; // class Environment

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
