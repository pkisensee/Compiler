///////////////////////////////////////////////////////////////////////////////
//
//  Value.cpp
//
//  Copyright � Pete Isensee (PKIsensee@msn.com).
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

#include <format>

#include "CompilerError.h"
#include "StrUtil.h"
#include "Value.h"

using namespace PKIsensee;

///////////////////////////////////////////////////////////////////////////////
//
// Convert any Value to a string

std::string Value::ToString() const
{
  switch( GetType() )
  {
  case ValueType::Str:  return GetString();
  case ValueType::Int:  return Util::ToStr<std::string>( GetInt() );
  case ValueType::Char: return Util::ToStr<std::string>( GetChar() );
  case ValueType::Bool: return GetBool() ? "true" : "false";
  }
  return {};
}

///////////////////////////////////////////////////////////////////////////////
//
// Convert any Value to an int

int Value::ToInt() const
{
  switch( GetType() )
  {
  case ValueType::Int:  return GetInt();
  case ValueType::Char: return GetChar();
  case ValueType::Bool: return GetBool();
  case ValueType::Str:
    {
      std::string str = GetString();
      if( !StrUtil::IsNumeric( str ) )
        throw CompilerError( 
          std::format("string '{}' cannot be interpreted as an integer", str) );
      return Util::ToNum<int>( str );
    }
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// Convert any Value to a char

char Value::ToChar() const
{
  switch( GetType() )
  {
  case ValueType::Int:  return static_cast<char>( GetInt() );
  case ValueType::Char: return GetChar();
  case ValueType::Bool: return GetBool() ? '1' : '0';
  case ValueType::Str:
    {
      std::string str = GetString();
      return str.empty() ? '\0' : str[0]; // first character
    }
  }
  return '\0';
}

///////////////////////////////////////////////////////////////////////////////
//
// True if Value has a "true" state
// 
// Str: true if string is not empty
// Int: true if int is not zero
// Char: true if char is not zero
// Bool: true if bool is true (duh)

bool Value::ToBool() const
{
  switch( GetType() )
  {
  case ValueType::Str:  return !GetString().empty();
  case ValueType::Int:  return  GetInt() != 0;
  case ValueType::Char: return  GetChar() != '\0';
  case ValueType::Bool: return  GetBool();
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
//
// Convert any Value to its negative counterpart. Boolean values are flipped.

Value Value::GetNegativeValue() const
{
  switch( GetType() )
  {
    case ValueType::Int:  return Value{ -GetInt() };
    case ValueType::Char: return Value{ -GetChar() };
    case ValueType::Bool: return Value{ !GetBool() };
    case ValueType::Str:
    {
      // empty: "" -> ""
      // leading dash: "-string" -> "+string"
      // leading plus: "+string" -> "-string"
      // everything else: "string" -> "-string"
      std::string str = GetString();
      if( str.empty() )
        return Value{ str };
      switch( str[0] )
      {
      case '-': 
        str[0] = '+';
        return Value{ str };
      case '+':
        str[0] = '-';
        return Value{ str };
      default:
        {
          std::string withLeadingDash{ "-" };
          withLeadingDash += str;
          return Value{ withLeadingDash };
        }
      }
    }
  }
  return {};
}

///////////////////////////////////////////////////////////////////////////////
//
// Unary x= operators
//
// Compiler errors (e.g. attempting to multiply a bool or string, attempting 
// division by zero) are handled at a higher layer

Value& Value::operator+=( const Value& rhs )
{
  switch( GetType() )
  {
  case ValueType::Str:
    std::get<std::string>( value_ ) += rhs.ToString();
    break;
  case ValueType::Int:
    std::get<int>( value_ ) += rhs.ToInt();
    break;
  case ValueType::Char:
    std::get<char>( value_ ) += rhs.ToChar();
    break;
  case ValueType::Bool:
    throw CompilerError{ "Can't add to bool" };
  }
  return *this;
}

Value& Value::operator-=( const Value& rhs )
{
  switch( GetType() )
  {
  case ValueType::Int:
    std::get<int>( value_ ) -= rhs.ToInt();
    break;
  case ValueType::Char:
    std::get<char>( value_ ) -= rhs.ToChar();
    break;
  case ValueType::Str:
    throw CompilerError{ std::format( "Can't subtract from string '{}'", GetString() ) };
  case ValueType::Bool:
    throw CompilerError{ "Can't subtract from bool" };
  }
  return *this;
}

Value& Value::operator*=( const Value& rhs )
{
  switch( GetType() )
  {
  case ValueType::Int:
    std::get<int>( value_ ) *= rhs.ToInt();
    break;
  case ValueType::Char:
    std::get<char>( value_ ) *= rhs.ToChar();
    break;
  case ValueType::Str:
    throw CompilerError{ std::format( "Can't multiply string '{}'", GetString() ) };
  case ValueType::Bool:
    throw CompilerError{ "Can't multiply bool" };
  }
  return *this;
}

Value& Value::operator/=( const Value& rhs )
{
  switch( GetType() )
  {
  case ValueType::Int:
  {
    int rhsValue = rhs.ToInt();
    if( rhsValue == 0 )
      throw CompilerError{ "Division by zero" };
    std::get<int>( value_ ) /= rhsValue;
    break;
  }
  case ValueType::Char:
  {
    char rhsValue = rhs.ToChar();
    if( rhsValue == '\0' )
      throw CompilerError{ "Division by zero" };
    std::get<char>( value_ ) /= rhsValue;
    break;
  }
  case ValueType::Str:
    throw CompilerError{ std::format( "Can't divide string '{}'", GetString() ) };
  case ValueType::Bool:
    throw CompilerError{ "Can't divide bool" };
  }
  return *this;
}

///////////////////////////////////////////////////////////////////////////////