///////////////////////////////////////////////////////////////////////////////
//
//  Function.h
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
#include <memory>
#include <string_view>

namespace PKIsensee
{

class Chunk;

class Function
{
public:

  Function();

  std::string_view GetName() const
  {
    return name_;
  }

  Chunk* GetChunk() const
  {
    return chunk_.get();
  }

  std::strong_ordering operator<=>( const Function& ) const;
  bool operator==( const Function& ) const;

private:
  std::shared_ptr<Chunk> chunk_; // TODO unique_ptr
  std::string_view name_;
  uint32_t paramCount_ = 0u;

}; // class Function

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
