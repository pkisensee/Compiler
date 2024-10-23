///////////////////////////////////////////////////////////////////////////////
//
//  Callable.cpp
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

#include "Callable.h"

using namespace PKIsensee;

Value Callable::Invoke( const Interpreter& interpreter, const ArgValues& arguments ) const
{
  return func_( interpreter, arguments );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
