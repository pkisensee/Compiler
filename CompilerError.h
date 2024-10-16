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
#include <cassert>
#include <string>

#include "Token.h"

namespace PKIsensee
{

///////////////////////////////////////////////////////////////////////////////
//
// Returns thrown compiler errors to the error handler

class CompilerError : public std::exception
{
private:

  Token token_;

  // Use stack rather than std::string to avoid throwing other exceptions
  static constexpr size_t kErrorMsgSize = 2048;
  char errorMsg_[kErrorMsgSize];

public:

  CompilerError() = delete;

  CompilerError( const char* errorMsg, Token token = Token{} ) :
    CompilerError( std::string_view( errorMsg ), token )
  {
  }

  CompilerError( std::string_view errorMsg, Token token = Token{} ) :
    token_{ token }
  {
    size_t i = 0u;
    for( ; i < errorMsg.size() && i < kErrorMsgSize - 1; ++i )
      errorMsg_[i] = errorMsg[i];
    errorMsg_[i] = '\0';
  }

  virtual const char* what() const noexcept override
  {
    return errorMsg_;
  }

  void SetToken( Token token )
  {
    token_ = token;
  }

  std::string_view GetErrorMessage()
  {
    // Find end of current message
    size_t i = 0u;
    for( ; errorMsg_[i] && i < kErrorMsgSize - 1; ++i )
      ;

    if( token_.GetType() != TokenType::EndOfFile )
    {
      // Append token info
      const char forToken[] = " for token '";
      for( size_t j = 0u; forToken[j] && i < kErrorMsgSize - 1; ++i, ++j )
        errorMsg_[i] = forToken[j];

      std::string_view tokenValue = token_.GetValue();
      for( size_t j = 0u; j < tokenValue.size() && i < kErrorMsgSize - 1; ++i, ++j )
        errorMsg_[i] = tokenValue[j];

      if( i < kErrorMsgSize - 1 )
        errorMsg_[i++] = '\'';
    }
    else
    {
      // Append EOF message
      const char atEOF[] = " at end of source";
      for( size_t j = 0u; atEOF[j] && i < kErrorMsgSize - 1; ++i, ++j )
        errorMsg_[i] = atEOF[j];
    }
  errorMsg_[i] = '\0';
  return std::string_view{ errorMsg_, i };
  }

}; // class CompilerError

} // namespace PKIsensee

////////////////////////////////////////////////////////////////////////////////////////////////////
