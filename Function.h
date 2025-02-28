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

  void SetName( std::string_view name )
  {
    name_ = name;
  }

  Chunk* GetChunk()
  {
    Chunk* chunk = chunk_.get();
    assert( chunk != nullptr );
    return chunk;
  }

  const Chunk* GetChunk() const
  {
    const Chunk* chunk = chunk_.get();
    assert( chunk != nullptr );
    return chunk;
  }

  uint32_t GetParamCount() const
  {
    return paramCount_;
  }

  void IncrementParamCount()
  {
    ++paramCount_;
  }

  std::strong_ordering operator<=>( const Function& ) const;
  bool operator==( const Function& ) const;

private:
  std::shared_ptr<Chunk> chunk_; // TODO unique_ptr
  std::string_view name_;
  uint32_t paramCount_ = 0u; // TODO argCount?

}; // class Function

class NativeFunction
{
public:

  typedef Value( *NativeFn )( uint32_t argCount, Value* args ); // TODO modernize; std::span for args?

  NativeFunction() = delete;
  NativeFunction( NativeFn fn, std::string_view name, uint32_t paramCount = 0 ) :
    function_( fn ),
    name_( name ),
    paramCount_( paramCount )
  {
  }

  NativeFn GetFunc() const
  {
    return function_;
  }

  std::string_view GetName() const
  {
    return name_;
  }

  uint32_t GetParamCount() const
  {
    return paramCount_;
  }

  std::strong_ordering operator<=>( const NativeFunction& ) const;
  bool operator==( const NativeFunction& ) const;

private:

  NativeFn function_;
  std::string_view name_;
  uint32_t paramCount_ = 0u;

}; // class NativeFunction

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
