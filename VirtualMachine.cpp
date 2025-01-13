///////////////////////////////////////////////////////////////////////////////
//
//  VirtualMachine.cpp
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

#include <cassert>
#include <functional>
#include <string_view>

#include "Chunk.h"
#include "Compiler.h"
#include "Value.h"
#include "VirtualMachine.h"

using namespace PKIsensee;

void VirtualMachine::Reset()
{
  //chunk_ = nullptr; TODO remove
  //ip_ = nullptr;
  stack_.clear();
  globals_.clear();
}

InterpretResult VirtualMachine::Interpret( std::string_view source )
{
  Compiler compiler; // TODO can we just use compiler_?
  auto main = compiler.Compile( source );
  Push( Value(main) );
  Chunk* chunk = main.GetChunk();
  CallFrame frame( main, chunk->GetCode(), &(stack_[0]) );
  frames_.push( frame );
  Call( main, 0 );
  return Run(); // TODO catch CompilerError
}

void VirtualMachine::Interpret( const Chunk* chunk )
{
  assert( chunk != nullptr );
  //chunk_ = chunk; TODO
  //ip_ = chunk_->GetCode();
  Run();
}

InterpretResult VirtualMachine::Run() // private
{
  CallFrame& frame = frames_.top();
  for( ;; )
  {
    Chunk* chunk = frame.GetFunction().GetChunk();
#if defined(DEBUG_TRACE_EXECUTION)
    std::cout << "          ";
    for( Value slot : stack_ )
      std::cout << "[ " << slot << " ]";
    std::cout << '\n';
    uint32_t offset = static_cast<uint32_t>( frame.GetIP() - chunk->GetCode() );
    chunk->DisassembleInstruction(offset);
#endif
    uint8_t instruction = ReadByte();
    OpCode opCode = static_cast<OpCode>( instruction );
    switch( opCode )
    {
    case OpCode::Constant: 
    {
      uint8_t index = ReadByte();
      Value constant = chunk->GetConstant( index );
      Push( constant );
      break;
    }
    case OpCode::True:
      Push( Value{ true } );
      break;
    case OpCode::False:
      Push( Value{ false } );
      break;
    case OpCode::Empty:
      Push( Value{ 0 } );
      break;
    case OpCode::Pop:
      Pop();
      break;
    case OpCode::GetLocal:
    {
      uint8_t index = ReadByte();
      if( index >= stack_.size() )
        throw CompilerError( "Referencing local variable that is out of scope" );
      const Value& local = frame.GetSlot( index );
      //const Value& local = stack_[index]; TODO remove
      Push( local );
      break;
    }
    case OpCode::SetLocal:
    {
      uint8_t index = ReadByte();
      assert( index < stack_.size() );
      frame.SetSlot( index, Peek() );
      // stack_[index] = Peek(); TODO remove
      break;
    }
    case OpCode::GetGlobal:
    {
      std::string varName = ReadString(); // key
      auto entry = globals_.find( varName );
      if( entry == std::end( globals_ ) )
        throw CompilerError( std::format( "Undefined variable '{}'", varName ) );
      const auto& [key, value] = *entry;
      Push( value );
      break;
    }
    case OpCode::DefineGlobal:
    {
      std::string varName = ReadString(); // key
      Value value = Peek(); // value
      globals_.insert( { varName, value } );
      Pop();
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
      std::cout << Pop();
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
      return true;
    }
  }
}

void VirtualMachine::Push( Value value )
{
  stack_.push( value );
}

Value VirtualMachine::Pop()
{
  assert( !stack_.empty() );
  Value top = stack_.top();
  stack_.pop();
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

bool VirtualMachine::CallValue( const Value& callee, uint8_t argCount )
{
  if( callee.GetType() != ValueType::Func2 )
    throw CompilerError( "Can only call functions" );

  return Call( callee.GetFunc2(), argCount );
}

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
  auto functionIndex = stack_.size() - argCount - 1;
  Value* slots = &(stack_[functionIndex]);
  // TODO use Peek above

  uint8_t* ip = function.GetChunk()->GetCode();
  CallFrame frame( function, ip, slots );
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
