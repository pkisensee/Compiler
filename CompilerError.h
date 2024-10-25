///////////////////////////////////////////////////////////////////////////////
//
//  CompilerError.h
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
#include <string_view>

#include "Token.h"

namespace PKIsensee
{

///////////////////////////////////////////////////////////////////////////////
//
// Returns thrown compiler errors to the error handler

class CompilerError : public std::exception
{
public:

  CompilerError() = delete;

  CompilerError( const char* errorMsg, Token token = Token{} ) :
    CompilerError( std::string_view( errorMsg ), token )
  {
  }

  CompilerError( std::string_view errorMsg, Token = Token{} );

  virtual const char* what() const noexcept override
  {
    return errorMsg_;
  }

  void SetToken( Token token )
  {
    token_ = token;
  }

  std::string_view GetErrorMessage();

private:

  Token token_;

  // Use stack rather than std::string to avoid throwing other exceptions
  static constexpr size_t kErrorMsgSize = 2048;
  char errorMsg_[kErrorMsgSize];

}; // class CompilerError

} // namespace PKIsensee

////////////////////////////////////////////////////////////////////////////////////////////////////
