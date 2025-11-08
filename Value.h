///////////////////////////////////////////////////////////////////////////////
//
//  Value.h
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
#include <compare>
#include <format>
#include <ostream>
#include <string>
#include <string_view>
#include <variant>

#include "Function.h"
#include "Token.h"

#include "..\frozen\unordered_map.h"

#pragma warning(push)
#pragma warning(disable: 4062)

namespace PKIsensee
{

enum class ValueType
{
  Min = 0,
  Str = Min, // Order must match std::variant below
  Int,
  Char,
  Bool,
  Func,
  NativeFunc,
  Closure,
  Max
}; // enum ValueType

constexpr size_t kMaxValueTypes = static_cast<size_t>( ValueType::Max );
constexpr frozen::unordered_map< ValueType, std::string_view, kMaxValueTypes >
kValueTypes =
{
  { ValueType::Str,  "Str" },
  { ValueType::Int,  "Int" },
  { ValueType::Char, "Char" },
  { ValueType::Bool, "Bool" },
  { ValueType::Func, "Func" },
  { ValueType::NativeFunc, "NtvFn" },
  { ValueType::Closure, "Clos" },
};

///////////////////////////////////////////////////////////////////////////////
//
// Holds a single value of ValueType

class Value
{
public:

  Value() = default;
  explicit Value( const std::string& str) :
    value_( str )
  {
  }
  explicit Value( std::string_view str ) :
    value_( std::string( str ) )
  {
  }
  explicit Value( const char* str ) :
    value_( std::string( str ) )
  {
  }
  explicit Value( int i ) :
    value_( static_cast<int64_t>(i) )
  {
  }
  explicit Value( int64_t i ) :
    value_( i )
  {
  }
  explicit Value( char c ) :
    value_( c )
  {
  }
  explicit Value( bool b ) :
    value_( b )
  {
  }
  explicit Value( Function fn ) :
    value_( fn )
  {
  }
  explicit Value( NativeFunction fn ) :
    value_( fn )
  {
  }
  explicit Value( Closure closure ) :
    value_( closure )
  {
  }
  explicit Value( Token );

  Value( const Value& ) = default;
  Value( Value&& ) = default;
  Value& operator=( const Value& ) = default;
  Value& operator=( Value&& ) = default;

  ValueType GetType() const
  {
    ValueType valueType = static_cast<ValueType>( value_.index() );
    assert( valueType >= ValueType::Min );
    assert( valueType < ValueType::Max );
    return valueType;
  }

  std::string_view GetTypeName() const
  {
    return kValueTypes.at( GetType() );
  }

  std::string GetString() const
  {
    return std::get<std::string>( value_ );
  }

  int64_t GetInt() const
  {
    return std::get<int64_t>( value_ );
  }

  char GetChar() const
  {
    return std::get<char>( value_ );
  }

  bool GetBool() const
  {
    return std::get<bool>( value_ );
  }

  Function GetFunc() const
  {
    return std::get<Function>( value_ );
  }

  NativeFunction GetNativeFunction() const
  {
    return std::get<NativeFunction>( value_ );
  }

  // TODO all of these Getters should return by reference for speed
  // It's key that GetClosure return by reference since the vector of upvalues
  // that it stores isn't copied, else need to replace the vector with a shared_ptr
  const Closure& GetClosure() const
  {
    return std::get<Closure>( value_ );
  }

  Closure& GetClosure()
  {
    return std::get<Closure>( value_ );
  }

  std::string ToString() const;
  int64_t ToInt() const;
  char ToChar() const;
  bool IsTrue() const;

  auto operator<=>( const Value& ) const = default;

  Value operator-() const;
  bool operator!() const;
  Value& operator+=( const Value& );
  Value& operator-=( const Value& );
  Value& operator*=( const Value& );
  Value& operator/=( const Value& );
  Value& operator%=( const Value& );

  friend std::ostream& operator<<( std::ostream&, const Value& );

private:
  std::variant<std::string, int64_t, char, bool, 
    Function, NativeFunction, Closure> value_;

}; // class Value

///////////////////////////////////////////////////////////////////////////////
//
// Arithmetic operators for Values

inline Value operator+( const Value& lhs, const Value& rhs )
{
  Value result = lhs;
  result += rhs;
  return result;
}

inline Value operator-( const Value& lhs, const Value& rhs )
{
  Value result = lhs;
  result -= rhs;
  return result;
}

inline Value operator*( const Value& lhs, const Value& rhs )
{
  Value result = lhs;
  result *= rhs;
  return result;
}

inline Value operator/( const Value& lhs, const Value& rhs )
{
  Value result = lhs;
  result /= rhs;
  return result;
}

inline Value operator%( const Value& lhs, const Value& rhs )
{
  Value result = lhs;
  result %= rhs;
  return result;
}

///////////////////////////////////////////////////////////////////////////////
//
// Boolean operators for Values

inline Value operator&&( const Value& lhs, const Value& rhs )
{
  bool result = lhs.IsTrue() && rhs.IsTrue();
  return Value{ result };
}

inline Value operator||( const Value& lhs, const Value& rhs )
{
  bool result = lhs.IsTrue() || rhs.IsTrue();
  return Value{ result };
}

///////////////////////////////////////////////////////////////////////////////
//
// Stream Value

inline std::ostream& operator<<( std::ostream& out, const Value& value )
{
  if( value.GetType() == ValueType::Str )
    out << '\"' << value.GetString() << '\"';
  else
    out << value.ToString();
  return out;
}

///////////////////////////////////////////////////////////////////////////////
//
// std::format Value

} // namespace PKIsensee

template <>
struct std::formatter<PKIsensee::Value>
{
  constexpr auto parse( std::format_parse_context& ctx )
  {
    return ctx.begin();
  }

  auto format( const PKIsensee::Value& value, std::format_context& ctx ) const
  {
    if( value.GetType() == PKIsensee::ValueType::Str )
      return std::format_to( ctx.out(), "\"{}\"", value.GetString() );
    else
      return std::format_to( ctx.out(), "{}", value.ToString() );
  }
};

#pragma warning(pop)

///////////////////////////////////////////////////////////////////////////////
