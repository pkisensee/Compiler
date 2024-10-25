///////////////////////////////////////////////////////////////////////////////
//
//  Environment.cpp
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

#include "CompilerError.h"
#include "Environment.h"
#include "Token.h"
#include "Value.h"

using namespace PKIsensee;

///////////////////////////////////////////////////////////////////////////////
//
// Assign the given value to the variable

void Environment::Assign( Token variable, const Value& value )
{
  // Update the value if it exists in this environment
  auto it = values_.find( variable.GetValue() );
  if( it != std::end( values_ ) )
  {
    auto& [key, foundValue] = *it;
    foundValue = value; // TODO check for change of type?
    return;
  }

  // If it doesn't exist in this environment, update the enclosing environment
  assert( enclosingEnv_ != nullptr ); // check to see if this ever happens TODO
  if( enclosingEnv_ != nullptr )
  {
    enclosingEnv_->Assign( variable, value );
    return;
  }

  throw CompilerError( std::format( "Undefined variable '{}'", variable.GetValue() ), variable );
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract the value of the variable

Value Environment::GetValue( Token variable ) const
{
  // Extract the value if it exists in this environment
  auto it = values_.find( variable.GetValue() );
  if( it != std::end( values_ ) )
  {
    const auto& [key, foundValue] = *it;
    return foundValue;
  }

  // Or get it from the enclosing environment
  assert( enclosingEnv_ != nullptr ); // check to see if this ever happens TODO
  if( enclosingEnv_ != nullptr )
    return enclosingEnv_->GetValue( variable );

  throw CompilerError( std::format( "Undefined variable '{}'", variable.GetValue() ), variable );
}

///////////////////////////////////////////////////////////////////////////////
