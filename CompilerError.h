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
#include <string>

namespace PKIsensee
{

class CompilerError
{
public:
  CompilerError() = delete;
  explicit CompilerError( const std::string& errorMsg ) :
    errorMsg_( errorMsg )
  {
  }

  std::string GetErrorMessage() const
  {
    return errorMsg_;
  }

private:
  std::string errorMsg_;
};

} // namespace PKIsensee

////////////////////////////////////////////////////////////////////////////////////////////////////
