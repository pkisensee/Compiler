///////////////////////////////////////////////////////////////////////////////
//
//  DynArray.h
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
#include <malloc.h>
#include <cstdint>
#include "CompilerError.h"

namespace PKIsensee
{

namespace { // anonymous

uint32_t GrowCapacity( uint32_t current ) // TODO GetNewCapacity
{
  return ( current < 8 ) ? 8 : ( current * 2 );
}

template<typename T>
T* GrowArray( void* p, uint32_t newCapacity ) // TODO GrowBlock
{
  // when p is nullptr, behavior is the same as calling malloc
  auto newSize = newCapacity * sizeof( T );
  _heapchk();
  void* result = std::realloc( p, newSize );
  if( result == NULL )
    throw CompilerError{ "Insufficient Memory" };
  _heapchk();
  return static_cast<T*>( result );
}

void FreeArray( void* p ) // TODO FreeBlock
{
  _heapchk();
  std::free( p );
}

} // anonymous

template<typename T>
class DynArray
{
public:
  DynArray() = default;
  ~DynArray()
  {
    FreeArray( data_ );
  }

  uint32_t GetCount() const // TODO GetSize
  {
    return count_;
  }

  T Get(uint32_t offset) const
  {
    assert( offset < count_ );
    return data_[offset];
  }

  const T* GetPtr() const
  {
    return data_;
  }

  T* GetPtr()
  {
    return data_;
  }

  void Append( T value )
  {
    // TODO count_ -> size_
    // TODO code_ -> bytecode_
    if( capacity_ < count_ + 1 )
    {
      capacity_ = GrowCapacity( capacity_ );
      data_ = GrowArray<T>( data_, capacity_ );
    }
    data_[count_] = value;
    _heapchk();
    ++count_;
  }

  void Free()
  {
    FreeArray( data_ );
    count_ = 0u;
    capacity_ = 0u;
    data_ = nullptr;
  }

  // Disable copy/move
  DynArray( const DynArray& ) = delete;
  DynArray& operator=( const DynArray& ) = delete;
  DynArray( DynArray&& ) = delete;
  DynArray& operator=( DynArray&& ) = delete;

private:
  uint32_t count_ = 0u;
  uint32_t capacity_ = 0u;
  T* data_ = nullptr;
};

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
