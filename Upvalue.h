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
#include <string>

class Upvalue
{
public:
  Upvalue() = default;

  explicit Upvalue( const void* slot ) : // TODO Value* slot
    location_( slot )
  {
  }

  //void SetLocation( const void* slot )
  //{
  //  location_ = slot;
  //}

  const void* GetLocation() const
  {
    return location_;
  }

  std::string GetName() const;
  std::strong_ordering operator<=>( const Upvalue& ) const;
  bool operator==( const Upvalue& ) const;

private:
  // const Value* location_; // location of capture; TODO rename capture_?
  const void* location_ = nullptr; // always points to a Value, but workaround to avoid recursive header issues TODO fix

}; // class Upvalue

///////////////////////////////////////////////////////////////////////////////
