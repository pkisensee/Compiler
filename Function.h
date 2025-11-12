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
#include <vector>

#include "CompilerError.h"

namespace PKIsensee
{

class ByteCodeBlock;
class Value;

class Function
{
public:

  static constexpr uint32_t kMaxParams = 32;
  static constexpr uint32_t kMaxUpvalues = 32;

  Function();

  std::string_view GetName() const
  {
    return name_;
  }

  void SetName( std::string_view name )
  {
    name_ = name;
  }

  ByteCodeBlock* GetByteCodeBlock()
  {
    ByteCodeBlock* byteCodeBlock = byteCodeBlock_.get();
    assert( byteCodeBlock != nullptr );
    return byteCodeBlock;
  }

  const ByteCodeBlock* GetByteCodeBlock() const
  {
    const ByteCodeBlock* byteCodeBlock = byteCodeBlock_.get();
    assert( byteCodeBlock != nullptr );
    return byteCodeBlock;
  }

  uint32_t GetParamCount() const
  {
    return paramCount_;
  }

  void IncrementParamCount()
  {
    ++paramCount_;
    if ( paramCount_ == kMaxParams )
      throw CompilerError( std::format( "Parameter count on function '{}' can't exceed '{}'", 
                                        name_, kMaxParams ) );
  }

  uint32_t GetUpvalueCount() const
  {
    return upvalueCount_;
  }

  void IncrementUpvalueCount()
  {
    ++upvalueCount_;
    if ( upvalueCount_ == kMaxUpvalues )
      throw CompilerError( std::format( "Too many closure variables in function; can't exceed '{}'", kMaxUpvalues ) );
  }

  std::strong_ordering operator<=>( const Function& ) const;
  bool operator==( const Function& ) const;

private:

  // Optimized for size because stored in std::variant in class Value
  std::shared_ptr<ByteCodeBlock> byteCodeBlock_; // TODO unique_ptr
  std::string_view name_;
  uint8_t paramCount_ = 0u;
  uint8_t upvalueCount_ = 0u;

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

class Closure
{
public:

  Closure() = default; // TODO = delete?

  Closure( const Closure& ) = default;
  Closure( Closure&& ) = default;
  Closure& operator=( const Closure& ) = default;
  Closure& operator=( Closure&& ) = default;

  explicit Closure( Function func ) :
    func_( func ),
    upvalues_( func.GetUpvalueCount() )
  {
  }

  std::string_view GetName() const
  {
    return func_.GetName();
  }

  const Function& GetFunction() const
  {
    return func_;
  }

  uint32_t GetUpvalueCount() const
  {
    return func_.GetUpvalueCount();
  }

  // Ideally Value would be passed by value to the following, however:
  // TODO Value.h includes Function.h, so Function.h can't include Value.h
  // Need to find a better solution, but pointers for now
  Value* GetUpvalue( uint8_t slotIndex ) const
  {
    assert( slotIndex < upvalues_.size() );
    assert( upvalues_[slotIndex].get() != nullptr );
    return upvalues_[slotIndex].get();
  }
    
  void SetUpvalue( uint8_t slotIndex, Value* slot )
  {
    assert( slotIndex < upvalues_.size() );
    assert( slot != nullptr );
    upvalues_[slotIndex] = std::make_shared<Value>(*slot);
  }

  std::strong_ordering operator<=>( const Closure& ) const;
  bool operator==( const Closure& ) const;

private:

  // TODO can we reduce this size, since Closure is used in Value, and
  // minimizing the size of Value improves efficiency?
  Function func_;
  std::vector<std::shared_ptr<Value>> upvalues_; // should this be a shared_ptr to std::vector? TODO

};

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
