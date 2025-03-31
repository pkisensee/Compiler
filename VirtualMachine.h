///////////////////////////////////////////////////////////////////////////////
//
//  VirtualMachine.h
//
//  Copyright � Pete Isensee (PKIsensee@msn.com).
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
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
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
  CallFrame( Function fn, uint8_t* ip, Value* slots, std::string_view* names ) :
    function_(fn),
    ip_(ip), // TODO should be span
    slots_(slots), // TODO should be span
    names_(names)
  {
    assert( ip != nullptr );
    assert( slots != nullptr );
    assert( names != nullptr );
  }

  Function GetFunction() const
  {
    return function_; 
  }

  const uint8_t* GetIP() const
  {
    return ip_;
  }

  uint8_t GetByte() const
  {
    return *ip_;
  }

  const Value& GetSlot( uint32_t index ) const
  {
    assert( slots_ != nullptr );
    // TODO validation on index
    return slots_[index];
  }

  std::string_view GetName( uint32_t index ) const
  {
    assert( names_ != nullptr );
    // TODO validation on index
    return names_[index];
  }

  void SetSlot( uint32_t index, const Value& value ) // TODO add name
  {
    // TODO validation on index
    slots_[index] = value;
    names_[index] = "";
  }

  void AdvanceIP( ptrdiff_t count = 1 )
  {
    ip_ += count;
  }

  void DisassembleInstruction() const
  {
    const Chunk* chunk = function_.GetChunk();
    uint32_t offset = static_cast<uint32_t>( GetIP() - chunk->GetCode() );
    chunk->DisassembleInstruction( offset, slots_, names_ );
  }

  uint8_t ReadByte()
  {
    auto value = GetByte();
    AdvanceIP();
    return value;
  }

  uint16_t ReadShort()
  {
    // Equivalent to:
    // uint8_t hi = ReadByte(); TODO test efficiency
    // uint8_t lo = ReadByte();
    const auto* ip = GetIP();
    auto hi = *ip++;
    auto lo = *ip;
    uint16_t value = static_cast<uint16_t>( ( hi << 8 ) | lo );
    AdvanceIP( 2 );
    return value;
  }

  std::string ReadString()
  {
    // define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
    // define READ_STRING() AS_STRING(READ_CONSTANT())
    // auto index = ReadByte();
    // Value value = chunk_->GetConstant( index );
    const auto index = GetByte();
    Value value = GetFunction().GetChunk()->GetConstant( index );
    AdvanceIP(); // TODO GetIPAndAdvance() ?
    return value.GetString();
  }

private:
  Function function_; // How slow is using this by value? TODO
  uint8_t* ip_ = nullptr;
  Value* slots_ = nullptr; // first location in stack_ that function can use
  std::string_view* names_ = nullptr; // slot names for debugging

};

class VirtualMachine
{
public:
  VirtualMachine();

  void Reset();
  InterpretResult Interpret( std::string_view source );
  void Interpret( const Chunk* );

  // Disable copy/move
  VirtualMachine( const VirtualMachine& ) = delete;
  VirtualMachine& operator=( const VirtualMachine& ) = delete;
  VirtualMachine( VirtualMachine&& ) = delete;
  VirtualMachine& operator=( VirtualMachine&& ) = delete;

  void DefineNativeFunctions();
  void DefineNative( NativeFunction );
  static Value ClockNative( uint32_t argCount, Value* args );
  static Value SquareNative( uint32_t argCount, Value* args );

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
    Push( binOp( lhs, rhs ), "BinaryOp" );
  }

  template< typename BinOp >
  void LogicalBinaryOp( BinOp binOp )
  {
    Value rhs = Pop();
    Value lhs = Pop();
    Push( Value{ binOp( lhs, rhs ) }, "LogicalBinaryOp" );
  }

  uint8_t ReadByte( CallFrame& );
  uint16_t ReadShort();
  std::string ReadString();
  InterpretResult Run();
  void Push( Value, std::string_view name );
  Value Pop();
  const Value& Peek( size_t = 0 ) const;
  bool CallValue( const Value& callee, uint8_t argCount );
  bool Call( Function, uint8_t argCount );
  void PrintStack();
  void PushFrame( Function, size_t index );

private:
  //Compiler compiler_; need this? TODO
  //const Chunk* chunk_ = nullptr;
  //const uint8_t* ip_ = nullptr; // instruction pointer
  // TODO replace array_stack with using aliases
  array_stack<CallFrame, kMaxCallFrames> frames_;
  array_stack<Value, kMaxStackValues> stack_;
  array_stack<std::string_view, kMaxCallFrames> names_; // for debugging
  std::unordered_map<std::string, Value> globals_;

};

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
