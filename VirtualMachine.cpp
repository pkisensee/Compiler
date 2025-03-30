///////////////////////////////////////////////////////////////////////////////
//
//  VirtualMachine.cpp
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

#include <cassert>
#include <chrono>
#include <functional>
#include <string_view>

#include "Chunk.h"
#include "Compiler.h"
#include "CompilerError.h"
#include "Value.h"
#include "VirtualMachine.h"

using namespace PKIsensee;

VirtualMachine::VirtualMachine()
{
  DefineNativeFunctions();
}

void VirtualMachine::Reset()
{
  //chunk_ = nullptr; TODO remove
  //ip_ = nullptr;
  frames_.clear();
  stack_.clear();
  names_.clear();
  globals_.clear();
  DefineNativeFunctions();
}

InterpretResult VirtualMachine::Interpret( std::string_view source )
{
  Compiler compiler; // TODO can we just use compiler_?
  Function main = compiler.Compile( source );
  Push( Value(main), "fn main" ); // add main to the stack
  Call( main, 0 );                // invoke main
  return Run(); // TODO catch CompilerError
}

void VirtualMachine::Interpret( const Chunk* chunk )
{
  assert( chunk != nullptr );
  //chunk_ = chunk; TODO
  //ip_ = chunk_->GetCode();
  Run();
}

void VirtualMachine::DefineNativeFunctions()
{
  // TODO Do we need to have the name in two places?
  DefineNative( NativeFunction{ ClockNative, "clock" } );
  DefineNative( NativeFunction{ SquareNative, "square", 1 } );
}

void VirtualMachine::DefineNative( NativeFunction function )
{
  std::string name{ function.GetName() };
  globals_.insert( { name, Value{ function } } );
}

// TODO if we end up with many of these, should add them to a separate file NativeFunctions.cpp

Value VirtualMachine::ClockNative( uint32_t, Value* ) // static TODO should be a span
{
  auto now = std::chrono::high_resolution_clock::now();
  // TODO safe alternative is to have Value contain time_point objects, but this function
  // is primarily here for testing, hence leave for now, or harden to avoid dangerous reinterpret_cast
  static_assert( sizeof( now ) == sizeof( int64_t ) );
  int64_t nowAsI64 = *( reinterpret_cast<int64_t*>( &now ) );
  return Value{ nowAsI64 };
}

Value VirtualMachine::SquareNative( uint32_t, Value* args ) // static
{
  return args[0] * args[0];
}

InterpretResult VirtualMachine::Run() // private
{
#if defined(DEBUG_TRACE_EXECUTION)
  const uint32_t kReadWidth = 25;
  const uint32_t kOutputWidth = 15;
  const std::string_view read = "ByteCode Read";
  const std::string_view output = "Output";
  const std::string_view stack = "Stack";
  std::cout << std::format( "\n{:<{}}{:<{}}{}\n", read, kReadWidth, output, kOutputWidth, stack );
#endif
  CallFrame& frame = frames_.top();
  for( ;; )
  {
#if defined(DEBUG_TRACE_EXECUTION)
    std::cout << std::format( "{:{}}", "", kReadWidth + kOutputWidth);
    for( Value slot : stack_ )
      std::cout << '[' << slot << ']';
    std::cout << '\n';
    frame.DisassembleInstruction();
#endif
    Chunk* chunk = frame.GetFunction().GetChunk();
    uint8_t instruction = ReadByte();
    OpCode opCode = static_cast<OpCode>( instruction );
    switch( opCode )
    {
    case OpCode::Constant: 
    {
      uint8_t index = ReadByte();
      Value constant = chunk->GetConstant( index );
      Push( constant, "const" );
      break;
    }
    case OpCode::True:
      Push( Value{ true }, "true" );
      break;
    case OpCode::False:
      Push( Value{ false }, "false" );
      break;
    case OpCode::Empty:
      Push( Value{ 0 }, "empty" );
      break;
    case OpCode::Pop:
      Pop();
      break;
    case OpCode::GetLocal:
    {
      uint8_t index = ReadByte();
      const Value& local = frame.GetSlot( index );
      std::string_view name = frame.GetName( index );
      Push( local, name );
      break;
    }
    case OpCode::SetLocal:
    {
      uint8_t index = ReadByte();
      frame.SetSlot( index, Peek() );
      break;
    }
    case OpCode::GetGlobal:
    {
      std::string varName = ReadString(); // key
      auto entry = globals_.find( varName );
      if( entry == std::end( globals_ ) )
        throw CompilerError( std::format( "Undefined variable '{}'", varName ) );
      const auto& [key, value] = *entry;
      Push( value, key ); // unlike varName, a local, key is safe because it points into globals_
      break;
    }
    case OpCode::DefineGlobal:
    {
      std::string varName = ReadString(); // key
      Value value = Pop(); // value at top of stack
      globals_.insert( { varName, value } );
      break;
    }
    case OpCode::SetGlobal:
    {
      std::string varName = ReadString(); // key
      auto entry = globals_.find( varName );
      if( entry == std::end( globals_ ) )
        throw CompilerError( std::format( "Undefined variable '{}'", varName ) );
      entry->second = Peek();
      break;
    }
    case OpCode::IsEqual:
      LogicalBinaryOp( std::equal_to<Value>() );
      // Same as (slower version):
      // Value rhs = Pop();
      // Value lhs = Pop();
      // Push( Value{ rhs == lhs } );
      break;
    case OpCode::Greater:
      LogicalBinaryOp( std::greater<Value>() );
      break;
    case OpCode::Less:
      LogicalBinaryOp( std::less<Value>() );
      break;
    case OpCode::Add:
      BinaryOp( std::plus<Value>() );
      break;
    case OpCode::Subtract:
      BinaryOp( std::minus<Value>() );
      break;
    case OpCode::Multiply:
      BinaryOp( std::multiplies<Value>() );
      break;
    case OpCode::Divide:
      BinaryOp( std::divides<Value>() );
      break;
    case OpCode::Negate:
      UnaryOp( std::negate<Value>() ); // Same as Push( -Pop() )
      break;
    case OpCode::Not:
      LogicalUnaryOp( std::logical_not<Value>() ); // Same as Push( Value{ !Pop() } )
      break;
    case OpCode::Print:
      std::cout << std::format( "{:{}}", "", kReadWidth );
      std::cout << Pop() << '\n';
      break;
    case OpCode::Jump:
    {
      auto jumpAhead = ReadShort();
      frame.AdvanceIP( jumpAhead );
      break;
    }
    case OpCode::JumpIfFalse:
    {
      auto jumpAhead = ReadShort();
      if( !Peek().IsTrue() )
        frame.AdvanceIP( jumpAhead );
      break;
    }
    case OpCode::Loop:
    {
      auto jumpBack = ReadShort();
      frame.AdvanceIP( -jumpBack );
      break;
    }
    case OpCode::Call:
    {
      auto argCount = ReadByte();
      if( !CallValue( Peek( argCount ), argCount ) )
        throw CompilerError( "Runtime error calling function" ); // TODO add fn name
      frame = frames_.top(); // get new frame on call stack
      break;
    }
    case OpCode::Return:
    {
      // Top of the stack contains the function return value (if any) or the function itself
      Value fnReturnValue = Pop();
      frames_.pop();
      if( frames_.empty() ) // exiting from main
      {
        stack_.clear();
        return true;
      }
      // vm.stackTop = frame->slots;
      // pop() from stack_ the equivalent of the argCount + locals? TODO
      // Discard all of the slots the callee was using for its parameters and local variables
      const Value* slots = &frame.GetSlot( 0 );
      while( !stack_.empty() )
      {
        if( &stack_.top() == slots )
          break;
        stack_.pop();
      }
      // Put function return value back on the stack
      Push( fnReturnValue, "fn return" );
      frame = frames_.top(); // get new frame on call stack
    }
    }
  }
}

void VirtualMachine::Push( Value value, std::string_view name )
{
  stack_.push( value );
  names_.push( name );
}

Value VirtualMachine::Pop()
{
  assert( !stack_.empty() );
  Value top = stack_.top();
  stack_.pop();
  names_.pop();
  return top;
}

const Value& VirtualMachine::Peek( size_t offset ) const
{
  // offset == 0: top of stack
  // offset == 1: one down from top, etc.
  assert( !stack_.empty() );
  assert( offset < stack_.size() );
  auto index = stack_.size() - offset - 1;
  return stack_[index];
}

#pragma warning(push)
#pragma warning(disable: 4061)

bool VirtualMachine::CallValue( const Value& callee, uint8_t argCount )
{
  switch( callee.GetType() )
  {
  case ValueType::Func2:
    return Call( callee.GetFunc2(), argCount );
  case ValueType::NativeFunc:
  {
    auto function = callee.GetNativeFunction();
    if( argCount != function.GetParamCount() )
      throw CompilerError( std::format( "Expected {} arguments to {} but received {}",
                           function.GetParamCount(), function.GetName(), argCount));

    // Index into first function argument
    // TODO args to std::span
    assert( argCount < stack_.size() );
    size_t nativeArgsIndex = stack_.size() - argCount;
    Value* args = ( argCount > 0 ) ? &stack_[nativeArgsIndex] : nullptr;

    // Invoke native function
    NativeFunction::NativeFn nativeFn = function.GetFunc(); // TODO Invoke()?
    Value result = nativeFn( argCount, args );

    // Remove arguments and native function from stack and append function result
    for( size_t i = 0; i < argCount; ++i )
      stack_.pop();
    stack_.pop();
    Push( result, "native fn result" );
    break;
  }
  default:
    throw CompilerError( "Can only call functions" );
  }
  return true; // TODO need result?
}

#pragma warning(pop)

// TODO void return?
bool VirtualMachine::Call( Function function, uint8_t argCount )
{
  if( argCount != function.GetParamCount() )
    throw CompilerError( std::format( "Expected {} arguments to {} but received {}", 
                                      function.GetParamCount(), function.GetName(), argCount ) );
  if( frames_.size() == frames_.capacity() )
    throw CompilerError( std::format( "Stack overflow; exceeded max function call depth of {}", 
                                      frames_.size() ) );

  // The function and its args are already on the stack, so back up to
  // point to the function itself
  assert( argCount < stack_.size() );
  size_t functionIndex = stack_.size() - argCount - 1;
  assert( functionIndex < stack_.capacity() ); // overflow check

  assert( functionIndex < stack_.size() );
  assert( functionIndex < names_.size() );

  Value* slots = &( stack_[functionIndex] ); // TODO stack_ to callStack_
  std::string_view* names = &( names_[functionIndex] );
  uint8_t* ip = function.GetChunk()->GetCode();
  CallFrame frame{ function, ip, slots, names };
  frames_.push( frame );

  return true;
}

void VirtualMachine::PrintStack()
{
  // TODO integrate this with compiler and runtime error handling
  for( const auto& frame : frames_ )
  {
    std::cout << frame.GetFunction().GetName() << '\n';
    // TODO line number
  }
}

void VirtualMachine::PushFrame( Function fn, size_t index )
{
  /**/
  assert( index < stack_.size() );
  assert( index < names_.size() );

  Value* slots = &( stack_[index] ); // TODO stack_ to callStack_
  std::string_view* names = &( names_[index] );
  uint8_t* ip = fn.GetChunk()->GetCode();
  CallFrame frame{ fn, ip, slots, names };
  frames_.push( frame );
}

uint8_t VirtualMachine::ReadByte() // private TODO ReadCode, NextByte ?
{
  CallFrame& frame = frames_.top();
  auto value = *frame.GetIP();
  frame.AdvanceIP();
  return value;
}

uint16_t VirtualMachine::ReadShort()
{
  //uint8_t hi = ReadByte();
  //uint8_t lo = ReadByte();
  CallFrame& frame = frames_.top();
  auto ip = frame.GetIP();
  auto hi = *ip++;
  auto lo = *ip;
  uint16_t value = static_cast<uint16_t>( ( hi << 8 ) | lo );
  frame.AdvanceIP( 2 );
  return value;
}

std::string VirtualMachine::ReadString()
{
  // define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
  // define READ_STRING() AS_STRING(READ_CONSTANT())
  // auto index = ReadByte();
  // Value value = chunk_->GetConstant( index );
  CallFrame& frame = frames_.top();
  auto index = *frame.GetIP();
  Value value = frame.GetFunction().GetChunk()->GetConstant( index );
  frame.AdvanceIP(); // TODO GetIPAndAdvance() ?
  return value.GetString();
}

///////////////////////////////////////////////////////////////////////////////
