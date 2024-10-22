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
#include <string_view>
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
  explicit Value( std::string_view str ) :
    value_( std::string(str) )
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

  std::string ToString() const;
  int ToInt() const;
  char ToChar() const;
  bool ToBool() const;
  Value GetNegativeValue() const;

  auto operator<=>( const Value& rhs ) const = default;

  Value& operator+=( const Value& rhs );
  Value& operator-=( const Value& rhs );
  Value& operator*=( const Value& rhs );
  Value& operator/=( const Value& rhs );

  friend std::ostream& operator<<( std::ostream&, const Value& );

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

///////////////////////////////////////////////////////////////////////////////
//
// Boolean operators for Values

inline Value operator&&( const Value& lhs, const Value& rhs )
{
  bool result = lhs.ToBool() && rhs.ToBool();
  return Value{ result };
}

inline Value operator||( const Value& lhs, const Value& rhs )
{
  bool result = lhs.ToBool() || rhs.ToBool();
  return Value{ result };
}

///////////////////////////////////////////////////////////////////////////////
//
// Stream Value

inline std::ostream& operator<<( std::ostream& out, const Value& value )
{
  if( value.GetType() == ValueType::Str )
    out << '\"' << value.ToString() << '\"';
  else
    out << value.ToString();
  return out;
}

} // namespace PKIsensee

#pragma warning(pop)

///////////////////////////////////////////////////////////////////////////////
