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
#include <string>
#include <variant>

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
  Max
}; // enum ValueType


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
  explicit Value( int i ) :
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

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Str: true if string is not empty
  // Int: true if int is not zero
  // Char: true if char is not zero
  // Bool: true if bool is true

  bool IsTrueEquivalent() const
  {
    if( value_.valueless_by_exception() )
      return false;

    switch( GetType() )
    {
    case ValueType::Str:  return !GetString().empty();
    case ValueType::Int:  return  GetInt() != 0;
    case ValueType::Char: return  GetChar() != '\0';
    case ValueType::Bool: return  GetBool();
    }
    return false;
  }

  ValueType GetType() const
  {
    ValueType valueType = static_cast<ValueType>( value_.index() );
    assert( valueType >= ValueType::Min );
    assert( valueType < ValueType::Max );
    return valueType;
  }

  std::string GetString() const
  {
    return std::get<std::string>( value_ );
  }

  int GetInt() const
  {
    return std::get<int>( value_ );
  }

  char GetChar() const
  {
    return std::get<char>( value_ );
  }

  bool GetBool() const
  {
    return std::get<bool>( value_ );
  }

  Value GetNegativeValue() const
  {
    switch( GetType() )
    {
    case ValueType::Int:  return Value{ -GetInt() };
    case ValueType::Char: return Value{ -GetChar() };
    }
    return {};
  }

  auto operator<=>( const Value& rhs ) const = default;

  Value& operator+=( const Value& rhs );
  Value& operator-=( const Value& rhs );
  Value& operator*=( const Value& rhs );
  Value& operator/=( const Value& rhs );

private:
  std::variant<std::string, int, char, bool> value_;

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

} // namespace PKIsensee

#pragma warning(pop)

///////////////////////////////////////////////////////////////////////////////

