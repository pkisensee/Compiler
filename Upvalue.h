///////////////////////////////////////////////////////////////////////////////
//
//  Upvalue.h
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

namespace PKIsensee
{

static constexpr uint32_t kMaxUpvalueIndex = 127;

class Upvalue
{
public:

  Upvalue() = default;
  Upvalue( const Upvalue& ) = default;
  Upvalue( Upvalue&& ) = default;
  Upvalue& operator=( const Upvalue& ) = default;
  Upvalue& operator=( Upvalue&& ) = default;

  Upvalue( bool local, uint32_t index ) :
    isLocal_( local )
  {
    SetIndex( index );
  }

  uint32_t GetIndex() const
  {
    return index_;
  }

  uint8_t GetIndexAsByte() const
  {
    // Safe since index can't exceed 7 bits
    return index_;
  }

  bool IsLocal() const
  {
    return isLocal_;
  }

  void SetIndex( uint32_t index )
  {
    if ( index > kMaxUpvalueIndex )
      throw CompilerError( std::format( "Can't exceed maximum upvalue index of {}", kMaxUpvalueIndex ) );
    index_ = static_cast<uint8_t>( index );
  }

private:
  uint8_t index_ : 7 = 0;
  uint8_t isLocal_ : 1 = false;

}; // class Upvalue

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////


