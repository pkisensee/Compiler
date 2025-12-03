///////////////////////////////////////////////////////////////////////////////
//
//  Local.h
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
#include "CompilerError.h"
#include "Token.h"

namespace PKIsensee
{

static constexpr uint32_t kMaxLocalDepth = 127;

class Local
{
public:

  Local()                           = default;
  Local( const Local& )             = default;
  Local( Local&& )                  = default;
  Local& operator=( const Local& )  = default;
  Local& operator=( Local&& )       = default;

  Local( Token token, uint32_t depth = 0 ) :
    token_{ token }
  {
    SetDepth( depth ); // checks for invalid depth
  }

  Token GetToken() const
  {
    return token_;
  }

  uint32_t GetDepth() const
  {
    return depth_;
  }

  bool IsInitialized() const
  {
    return isInitialized_;
  }

  void SetLocal( Token token, uint32_t depth )
  {
    token_ = token;
    SetDepth( depth );
  }

  void SetDepth( uint32_t depth )
  {
    if ( depth > kMaxLocalDepth )
      throw CompilerError( std::format( "Can't exceed local variable depth of {}", kMaxLocalDepth ) );
    depth_ = static_cast<uint8_t>( depth );
  }

  void SetInitialized( bool isInitialized )
  {
    isInitialized_ = isInitialized;
  }

private:
  Token   token_;
  uint8_t depth_ : 7 = 0;
  uint8_t isInitialized_ : 1 = false;

}; // class Local

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////

