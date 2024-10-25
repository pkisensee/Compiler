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
#include "CompilerError.h"
#include "Environment.h"
#include "Interpreter.h"
#include "Stmt.h"

using namespace PKIsensee;

///////////////////////////////////////////////////////////////////////////////
//
// Create a callable object from a function statement

Callable::Callable( const FuncStmt* declaration ) :
  declaration_{ declaration }
{
  assert( declaration != nullptr );
  paramCount_ = declaration->GetParams().size();
}

///////////////////////////////////////////////////////////////////////////////
//
// Invoke the callable object using the given interpreter's environment and
// the passed arguments

Value Callable::Invoke( const Interpreter& interpreter, const ArgValues& arguments ) const
{
  if( declaration_ == nullptr )
    return func_( interpreter, arguments );

  // Function parameters become local variables for the function
  const ParamList& params = declaration_->GetParams();
  assert( params.size() == arguments.size() );

  EnvPtr env = interpreter.GetGlobalsEnv();
  auto argIt = std::begin( arguments );
  for( const auto& [paramType, paramName] : params )
    env->Define( paramName.GetValue(), *argIt++ );

  // Execute the function and report the result
  try {
    interpreter.Execute( declaration_->GetBody(), env );
  }
  catch( ReturnException& returnEx ) {
    return returnEx.GetValue();
  }

  // Function reached the end of its body without hitting a return statement,
  // e.g. void function
  return {};
}

///////////////////////////////////////////////////////////////////////////////
//
// Callable objects can be stored in the Value type, which must be comparable.
// If a callable object is compared, we've encountered a compiler error.

std::strong_ordering Callable::operator<=>( const Callable& ) const
{
  throw CompilerError( "Can't compare against function object" );
}

bool Callable::operator==( const Callable& ) const
{
  throw CompilerError( "Can't compare against function object" );
}

///////////////////////////////////////////////////////////////////////////////
