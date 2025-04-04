///////////////////////////////////////////////////////////////////////////////
//
//  Upvalue.cpp
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
#include "Upvalue.h"
#include "Value.h"

using namespace PKIsensee;

std::string Upvalue::GetName() const
{
  if( location_ == nullptr )
    return {};

  const Value* value = reinterpret_cast<const Value*>( location_ );
  return value->ToString();
}

bool Upvalue::operator==( const Upvalue& ) const
{
  throw CompilerError( "Can't compare against upvalue object" );
}

std::strong_ordering Upvalue::operator<=>( const Upvalue& ) const
{
  throw CompilerError( "Can't compare against upvalue object" );
}

///////////////////////////////////////////////////////////////////////////////
