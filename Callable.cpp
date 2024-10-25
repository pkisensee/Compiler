///////////////////////////////////////////////////////////////////////////////
//
//  Callable.cpp
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

#include "Callable.h"
#include "Interpreter.h"

using namespace PKIsensee;

Value Callable::Invoke( const Interpreter& interpreter, const ArgValues& arguments ) const
{
  if( declaration_ == nullptr )
    return func_( interpreter, arguments );

  const ParamList& params = declaration_->GetParams();
  assert( params.size() == arguments.size() );

  EnvPtr env = interpreter.GetGlobalsEnv();
  auto argIt = std::begin( arguments );
  for( const auto& [paramType, paramName] : params )
    env->Define( paramName.GetValue(), *argIt++ );

  try {
    interpreter.Execute( declaration_->GetBody(), env );
  }
  catch( ReturnException& returnEx ) {
    return returnEx.GetValue();
  }
  return {};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
