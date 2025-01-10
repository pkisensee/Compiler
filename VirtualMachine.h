///////////////////////////////////////////////////////////////////////////////
//
//  VirtualMachine.h
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
#define DEBUG_TRACE_EXECUTION 1
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "array_stack.h"
#include "Compiler.h"
#include "Value.h"

namespace PKIsensee
{

using InterpretResult = bool; // TODO may want to track specific errors in future
class Chunk;

static constexpr uint32_t kMaxCallFrames = 64;
static constexpr uint32_t kMaxStackValues = kMaxCallFrames * 64; // TODO 16K too much?

class CallFrame
{
public:

  CallFrame() = default;
  CallFrame( Function fn, uint8_t* ip, Value* slots ) :
    function_(fn),
    ip_(ip),
    slots_(slots)
  {
  }

  Function GetFunction()
  {
    return function_; 
  }

  const uint8_t* GetIP() const
  {
    return ip_;
  }

  const Value& GetSlot( uint32_t index ) const
  {
    assert( slots_ != nullptr );
    // TODO validation on index
    return slots_[index];
  }

  void SetSlot( uint32_t index, const Value& value )
  {
    // TODO validation on index
    slots_[index] = value;
  }

  void AdvanceIP( ptrdiff_t count = 1 )
  {
    ip_ += count;
  }

private:
  Function function_; // How slow is using this by value? TODO
  uint8_t* ip_ = nullptr;
  Value* slots_ = nullptr; // first location in stack_ that function can use

};

class VirtualMachine
{
public:
  VirtualMachine() = default;

  void Reset();
  InterpretResult Interpret( std::string_view source );
  void Interpret( const Chunk* );

  // Disable copy/move
  VirtualMachine( const VirtualMachine& ) = delete;
  VirtualMachine& operator=( const VirtualMachine& ) = delete;
  VirtualMachine( VirtualMachine&& ) = delete;
  VirtualMachine& operator=( VirtualMachine&& ) = delete;

private:

  template< typename UnaryOp >
  void UnaryOp( UnaryOp unaryOp )
  {
    assert( !stack_.empty() );
    stack_.top() = unaryOp( stack_.top() );
  }

  template< typename UnaryOp >
  void LogicalUnaryOp( UnaryOp logicalUnaryOp )
  {
    assert( !stack_.empty() );
    stack_.top() = Value{ logicalUnaryOp( stack_.top() ) };
  }

  template< typename BinOp >
  void BinaryOp( BinOp binOp )
  {
    Value rhs = Pop();
    Value lhs = Pop();
    Push( binOp( lhs, rhs ) );
  }

  template< typename BinOp >
  void LogicalBinaryOp( BinOp binOp )
  {
    Value rhs = Pop();
    Value lhs = Pop();
    Push( Value{ binOp( lhs, rhs ) } );
  }

  uint8_t ReadByte();
  uint16_t ReadShort();
  std::string ReadString();
  InterpretResult Run();
  void Push( Value );
  Value Pop();
  const Value& Peek( size_t = 0 ) const;
  bool CallValue( const Value& callee, uint8_t argCount );
  bool Call( Function, uint8_t argCount );

private:
  Compiler compiler_;
  //const Chunk* chunk_ = nullptr;
  //const uint8_t* ip_ = nullptr; // instruction pointer
  array_stack<CallFrame, kMaxCallFrames> frames_;
  array_stack<Value, kMaxStackValues> stack_;
  std::unordered_map<std::string, Value> globals_;

};

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
