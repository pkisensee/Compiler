///////////////////////////////////////////////////////////////////////////////
//
//  CompilerError.cpp
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

using namespace PKIsensee;

///////////////////////////////////////////////////////////////////////////////
//
// Error message on stack to avoid allocations

CompilerError::CompilerError( std::string_view errorMsg, Token token ) :
  token_{ token }
{
  size_t i = 0u;
  for( ; i < errorMsg.size() && i < kErrorMsgSize - 1; ++i )
    errorMsg_[i] = errorMsg[i];
  errorMsg_[i] = '\0';
}

///////////////////////////////////////////////////////////////////////////////
//
// Build the full error message including token information

std::string_view CompilerError::GetErrorMessage()
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

////////////////////////////////////////////////////////////////////////////////////////////////////
