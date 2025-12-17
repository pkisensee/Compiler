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
#include <functional>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

#include "CompilerError.h"

namespace PKIsensee
{

class ByteCodeBlock;
class Value;

class FunctionBase
{
public:

  static constexpr uint32_t kMaxParams = 32;
  static constexpr uint32_t kMaxUpvalues = 32;

  // Function objects can be stored in the Value type, which must be comparable.
  auto operator<=>( const FunctionBase& ) const noexcept = default;
  bool operator==( const FunctionBase& ) const noexcept = default;

}; // class FunctionBase

///////////////////////////////////////////////////////////////////////////////

class Function : public FunctionBase
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

  // Function objects can be stored in the Value type, which must be comparable.
  auto operator<=>( const Function& ) const noexcept = default;
  bool operator==( const Function& ) const noexcept = default;

private:

  // Optimized for size because stored in std::variant in class Value
  std::shared_ptr<ByteCodeBlock> byteCodeBlock_; // TODO unique_ptr
  std::string_view name_;
  uint8_t paramCount_ = 0u;
  uint8_t upvalueCount_ = 0u;

}; // class Function

///////////////////////////////////////////////////////////////////////////////

class NativeFunction : public FunctionBase
{
public:

  // Prototype: Value foo( std::span<Value> );
  using NativeFn = std::function< Value( std::span<Value> ) >;

  NativeFunction() = delete;

  NativeFunction( std::string_view name, NativeFn fn, uint32_t argCount = 0 ) :
    function_( fn ),
    name_( name ),
    argCount_( static_cast<uint8_t>( argCount ) )
  {
    if ( argCount >= kMaxParams )
      throw CompilerError( std::format( "Argument count on function '{}' can't exceed '{}'",
        name_, kMaxParams ) );
  }

  NativeFn GetFunc() const
  {
    return function_;
  }

  std::string_view GetName() const
  {
    return name_;
  }

  uint32_t GetParamCount() const // TODO name change
  {
    return argCount_;
  }

  // Function objects can be stored in the Value type, which must be comparable
  bool operator==( const NativeFunction& rhs ) const noexcept
  {
    return name_ == rhs.name_;
  }

  std::partial_ordering operator<=>( const NativeFunction& rhs ) const noexcept
  {
    // Compiler can't synthesize "= default" implementation because function pointers
    // and std::function aren't relationally comparable using <, >, <=, >= or <=>. 
    // Avoid the comparison altogether and just compare names.

    auto compare = ( name_ <=> rhs.name_ );
    if ( compare != 0 )
      return compare;

    return ( argCount_ <=> rhs.argCount_ );
  }

private:

  // Optimized for size because stored in std::variant in class Value
  NativeFn function_;
  std::string_view name_;
  uint8_t argCount_ = 0;

}; // class NativeFunction

///////////////////////////////////////////////////////////////////////////////

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

  const Value& GetUpvalue( uint32_t slotIndex ) const
  {
    assert( slotIndex < upvalues_.size() );
    assert( upvalues_[slotIndex].get() != nullptr );
    return *upvalues_[slotIndex].get();
  }
    
  void SetUpvalue( uint32_t slotIndex, const Value& slot )
  {
    assert( slotIndex < upvalues_.size() );
    upvalues_[slotIndex] = std::make_shared<Value>(slot);
  }

  // Function objects can be stored in the Value type, which must be comparable.
  auto operator<=>( const Closure& ) const noexcept = default;
  bool operator==( const Closure& ) const noexcept = default;

private:

  Function func_;
  std::vector<std::shared_ptr<Value>> upvalues_;

};

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
