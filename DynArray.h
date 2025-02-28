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
#include <vector>

// TODO replace DynArray with std::vector and remove this file from the project

namespace PKIsensee
{

template<typename T>
class DynArray
{
public:
  DynArray() = default;

  uint32_t GetCount() const // TODO GetSize
  {
    return static_cast<uint32_t>( data_.size() );
  }

  T Get(uint32_t offset) const
  {
    return data_[offset];
  }

  const T* GetPtr() const
  {
    return data_.data();
  }

  T* GetPtr()
  {
    return data_.data();
  }

  void Append( T value )
  {
    data_.push_back( value );
  }

  void Free()
  {
    data_.clear();
  }

  // Disable copy/move
  DynArray( const DynArray& ) = delete;
  DynArray& operator=( const DynArray& ) = delete;
  DynArray( DynArray&& ) = delete;
  DynArray& operator=( DynArray&& ) = delete;

private:
  std::vector<T> data_;
};

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
