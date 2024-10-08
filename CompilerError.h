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

namespace PKIsensee
{

class CompilerError : public std::exception
{
private:
  // Use stack rather than std::string to avoid throwing other exceptions
  static constexpr size_t kErrorMsgSize = 2048;
  char errorMsg_[kErrorMsgSize];

public:
  CompilerError() = delete;
  explicit CompilerError( const char* errorMsg )
  {
    assert( errorMsg != nullptr );
    size_t i = 0u;
    for( ; errorMsg[i] && i < kErrorMsgSize-1; ++i )
      errorMsg_[i] = errorMsg[i];
    errorMsg_[i] = '\0';
  }

  virtual const char* what() const noexcept override
  {
    return errorMsg_;
  }

  std::string_view GetErrorMessage() const
  {
    return std::string_view{ errorMsg_ };
  }
};

} // namespace PKIsensee

////////////////////////////////////////////////////////////////////////////////////////////////////
