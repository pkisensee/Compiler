///////////////////////////////////////////////////////////////////////////////
//
//  Function.cpp
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

#include "ByteCodeBlock.h"
#include "CompilerError.h"
#include "Function.h"

using namespace PKIsensee;

Function::Function() :
  byteCodeBlock_{ std::make_shared<ByteCodeBlock>() }
{
}

///////////////////////////////////////////////////////////////////////////////
//
// Function objects can be stored in the Value type, which must be comparable.
// If a function object is compared, we've encountered a compiler error.

std::strong_ordering Function::operator<=>( const Function& ) const
{
  throw CompilerError( "Can't compare against function object" );
}

bool Function::operator==( const Function& ) const
{
  throw CompilerError( "Can't compare against function object" );
}

bool NativeFunction::operator==( const NativeFunction& ) const
{
  throw CompilerError( "Can't compare against function object" );
}

std::strong_ordering NativeFunction::operator<=>( const NativeFunction& ) const
{
  throw CompilerError( "Can't compare against function object" );
}

bool Closure::operator==( const Closure& ) const
{
  throw CompilerError( "Can't compare against closure object" );
}

std::strong_ordering Closure::operator<=>( const Closure& ) const
{
  throw CompilerError( "Can't compare against closure object" );
}

///////////////////////////////////////////////////////////////////////////////
