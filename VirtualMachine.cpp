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

#include "ByteCodeBlock.h"
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
  //byteCodeBlock_ = nullptr; TODO remove
  //ip_ = nullptr;
  frames_.clear();
  stack_.clear();
  names_.clear();
  globals_.clear();
  output_.clear();
  DefineNativeFunctions();
}

InterpretResult VirtualMachine::Interpret( std::string_view source )
{
  Compiler compiler; // TODO can we just use compiler_?
  Closure main{ compiler.Compile( source ) };
  Push( Value(main), "fn main" ); // add main to the stack
  Call( main, 0 );                // invoke main
  return Run(); // TODO catch CompilerError
}

void VirtualMachine::Interpret( [[maybe_unused]] const ByteCodeBlock* byteCodeBlock )
{
  assert( byteCodeBlock != nullptr );
  //byteCodeBlock_ = byteCodeBlock; TODO
  //ip_ = byteCodeBlock_->GetCode();
  Run();
}

void VirtualMachine::DefineNativeFunctions()
{
  // TODO Do we need to have the name in two places?
  DefineNative( NativeFunction{ ClockNative, "clock" } );
  DefineNative( NativeFunction{ SquareNative, "square", 1 } );
  DefineNative( NativeFunction{ GenreNative, "genre" } );
}

void VirtualMachine::DefineNative( NativeFunction function )
{
  std::string name{ function.GetName() };
  globals_.insert( { name, Value{ function } } );
}

// TODO if we end up with many of these, should add them to a separate file NativeFunctions.cpp

Value VirtualMachine::ClockNative( std::span<Value> /*args*/ ) // static
{
  auto now = std::chrono::high_resolution_clock::now();
  // TODO safe alternative is to have Value contain time_point objects, but this function
  // is primarily here for testing, hence leave for now, or harden to avoid dangerous reinterpret_cast
  static_assert( sizeof( now ) == sizeof( int64_t ) );
  int64_t nowAsI64 = *( reinterpret_cast<int64_t*>( &now ) );
  return Value{ nowAsI64 };
}

Value VirtualMachine::SquareNative( std::span<Value> args ) // static
{
  assert( args.size() == 1 );
  return args[0] * args[0];
}

Value VirtualMachine::GenreNative( std::span<Value> /*args*/ ) // static
{
  return Value{ "Rock" };
}

InterpretResult VirtualMachine::Run() // private
{
  const uint32_t kReadWidth = 25;
#if defined(DEBUG_TRACE_EXECUTION)
  const uint32_t kOutputWidth = 15;
  const std::string_view read = "ByteCode Read";
  const std::string_view output = "Output";
  const std::string_view stack = "Stack";
  std::cout << std::format( "\n{:<{}}{:<{}}{}\n", read, kReadWidth, output, kOutputWidth, stack );
#endif
  CallFrame* frame = &frames_.back();
  for( ;; )
  {
#if defined(DEBUG_TRACE_EXECUTION)
    std::cout << std::format( "{:{}}", "", kReadWidth + kOutputWidth);
    for( Value slot : stack_ )
      std::cout << '[' << slot << ']';
    std::cout << '\n';
    frame->DisassembleInstruction();
#endif
    ByteCodeBlock* byteCodeBlock = frame->GetFunction().GetByteCodeBlock();
    uint8_t instruction = frame->ReadByte();
    OpCode opCode = static_cast<OpCode>( instruction );
    switch( opCode )
    {
    case OpCode::Constant: 
    {
      uint8_t index = frame->ReadByte();
      Value constant = byteCodeBlock->GetConstant( index );
      Push( constant, "const" ); // TODO get actual name from table in future for debugging
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
      uint8_t index = frame->ReadByte();
      const Value& local = frame->GetSlot( index );
      std::string_view name = frame->GetName( index );
      Push( local, name );
      break;
    }
    case OpCode::SetLocal:
    {
      uint8_t index = frame->ReadByte();
      frame->SetSlot( index, Peek() );
      break;
    }
    case OpCode::GetGlobal:
    {
      std::string varName = frame->ReadString(); // key
      auto entry = globals_.find( varName );
      if( entry == std::end( globals_ ) )
        throw CompilerError( std::format( "Undefined variable '{}'", varName ) );
      const auto& [key, value] = *entry;
      Push( value, key ); // unlike varName, a local, key is safe because it points into globals_
      break;
    }
    case OpCode::DefineGlobal:
    {
      std::string varName = frame->ReadString(); // key
      Value value = Pop(); // value at top of stack
      globals_.insert( { varName, value } );
      break;
    }
    case OpCode::SetGlobal:
    {
      std::string varName = frame->ReadString(); // key
      auto entry = globals_.find( varName );
      if( entry == std::end( globals_ ) )
        throw CompilerError( std::format( "Undefined variable '{}'", varName ) );
      entry->second = Peek();
      break;
    }
    case OpCode::GetUpvalue:
    {
      auto upvalueSlotIndex = frame->ReadByte();
      const Value& upvalue = frame->GetClosure().GetUpvalue( upvalueSlotIndex );
      Push( upvalue, "upvalue" ); // &&& TODO store name with the value itself?
      break;
    }
    case OpCode::SetUpvalue:
    {
      uint8_t upvalueSlotIndex = frame->ReadByte();
      Value upvalue = Peek();
      frame->GetClosure().SetUpvalue( upvalueSlotIndex, upvalue );
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
    case OpCode::Modulus:
      BinaryOp( std::modulus<Value>() );
      break;
    case OpCode::Negate:
      UnaryOp( std::negate<Value>() ); // Same as Push( -Pop() )
      break;
    case OpCode::Not:
      LogicalUnaryOp( std::logical_not<Value>() ); // Same as Push( Value{ !Pop() } )
      break;
    case OpCode::Print:
    {
      std::cout << std::format( "{:{}}", "", kReadWidth );
      Value outValue = Pop();
      std::cout << outValue << '\n';

      // For debugging/validation
      if (!output_.empty())
        output_ += '\n';
      output_ += outValue.ToString();
      break;
    }
    case OpCode::Jump:
    {
      auto jumpAhead = frame->ReadShort();
      frame->AdvanceIP( jumpAhead );
      break;
    }
    case OpCode::JumpIfFalse:
    {
      auto jumpAhead = frame->ReadShort();
      if( !Peek().IsTrue() )
        frame->AdvanceIP( jumpAhead );
      break;
    }
    case OpCode::Loop:
    {
      auto jumpBack = frame->ReadShort();
      frame->AdvanceIP( -jumpBack );
      break;
    }
    case OpCode::Call:
    {
      auto argCount = frame->ReadByte();
      if( !CallValue( Peek( argCount ), argCount ) )
        throw CompilerError( "Runtime error calling function" ); // TODO add fn name
      frame = &frames_.back(); // get new frame on call stack
      break;
    }
    case OpCode::Closure:
    {
      // Store the closure object
      uint8_t closureIndex = frame->ReadByte();
      Value closureValue = byteCodeBlock->GetConstant( closureIndex );
      Closure& closure = closureValue.GetClosure();

      // Store the upvalues in the closure
      for( uint32_t i = 0u; i < closure.GetUpvalueCount(); ++i )
      {
        auto isLocal = frame->ReadByte();
        auto slotIndex = frame->ReadByte();
        if( isLocal )
        {
          Value capture = CaptureUpvalue( frame->GetSlot( slotIndex ) );
          closure.SetUpvalue( i, capture ); // TODO looks like using a stack-based variable, but the data is copied to a shared_ptr
        }
        else
          closure.SetUpvalue( i, frame->GetClosure().GetUpvalue( slotIndex ) );
      }
      Push( closureValue, closure.GetName() );

      break;
    }
    case OpCode::Return:
    {
      // Top of the stack contains the function return value
      Value fnReturnValue = Pop();
      if( frames_.size() == 1 ) // exiting from main
      {
        frames_.clear();
        stack_.clear();
        return true;
      }

      // Discard function and its arguments from the stack
      // TODO consider putting argCount in the frame data (e.g. frame.GetArgCount())
      const Value* slots = &frame->GetSlot( 0 );
      const Value* stackTop = &stack_.back();
      ptrdiff_t argCount = stackTop - slots;
      assert( argCount >= 0 );
      assert( argCount < 256 ); // TODO kMaxArgCount
      for( ptrdiff_t i = 0; i < argCount + 1; ++i )
        Pop();

      // Put function return value back on the stack
      Push( fnReturnValue, "fn return" );

      frames_.pop_back();
      frame = &frames_.back(); // use new frame on call stack
    }
    }
  }
}

void VirtualMachine::Push( Value value, std::string_view name )
{
  stack_.push_back( value );
  names_.push_back( name );
}

Value VirtualMachine::Pop()
{
  assert( !stack_.empty() );
  Value top = stack_.back();
  stack_.pop_back();
  names_.pop_back();
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
  case ValueType::Closure:
    return Call( callee.GetClosure(), argCount);
  //case ValueType::Func: // TODO does this go away with the introduction of closures?
  //  return Call( callee.GetFunc(), argCount );
  case ValueType::NativeFunc:
  {
    const auto& function = callee.GetNativeFunction();
    if( argCount != function.GetParamCount() )
      throw CompilerError( std::format( "Expected {} arguments to {} but received {}",
                           function.GetParamCount(), function.GetName(), argCount));

    // Index into first function argument
    // TODO args to std::span
    assert( argCount < stack_.size() );
    size_t nativeArgsIndex = stack_.size() - argCount;
    Value* args = ( argCount > 0 ) ? &stack_[nativeArgsIndex] : nullptr;
    auto argList = std::span<Value>{ args, argCount };

    // Invoke native function
    NativeFunction::NativeFn nativeFn = function.GetFunc(); // TODO Invoke()?
    Value result = nativeFn( argList );

    // Remove arguments and native function from stack and append function result
    for( size_t i = 0; i < argCount; ++i )
      stack_.pop_back();
    stack_.pop_back();
    Push( result, "native fn result" );
    break;
  }
  default:
    throw CompilerError( "Can only call functions" );
  }
  return true; // TODO need result?
}

#pragma warning(pop)

Value VirtualMachine::CaptureUpvalue( Value localValue )
{
  return localValue;
}

// TODO void return?
bool VirtualMachine::Call( Closure closure, uint8_t argCount )
{
  const Function& function = closure.GetFunction();
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
  auto ip = function.GetByteCodeBlock()->GetEntryPoint();
  CallFrame frame{ closure, ip.data(), slots, names};
  frames_.emplace_back( frame );

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
  auto ip = fn.GetByteCodeBlock()->GetEntryPoint();
  CallFrame frame{ Closure(fn), ip.data(), slots, names}; // pass ip as std::span
  frames_.emplace_back( frame );
}

///////////////////////////////////////////////////////////////////////////////
