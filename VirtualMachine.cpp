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
#include <functional>
#include <string_view>

#include "Chunk.h"
#include "Compiler.h"
#include "VirtualMachine.h"

using namespace PKIsensee;

InterpretResult VirtualMachine::Interpret( std::string_view source )
{
  Chunk chunk; // TODO can we just use chunk_?
  Compiler compiler;
  if( !compiler.Compile( source, &chunk ) )
    return false;

  chunk_ = &chunk;
  ip_ = chunk_->GetCode();
  return Run();
}

void VirtualMachine::Interpret( const Chunk* chunk )
{
  assert( chunk != nullptr );
  chunk_ = chunk;
  ip_ = chunk_->GetCode();
  Run();
}

InterpretResult VirtualMachine::Run() // private
{
  for( ;; )
  {
#if defined(DEBUG_TRACE_EXECUTION)
    std::cout << "          ";
    for( Value slot : stack_ )
      std::cout << "[ " << slot << " ]";
    std::cout << '\n';
    uint32_t offset = static_cast<uint32_t>( ip_ - chunk_->GetCode() );
    chunk_->DisassembleInstruction( offset );
#endif
    uint8_t instruction = ReadByte();
    OpCode opCode = static_cast<OpCode>( instruction );
    switch( opCode )
    {
    case OpCode::Constant: 
    {
      uint8_t index = ReadByte();
      Value constant = chunk_->GetConstant( index );
      Push( constant );
      break;
    }
    case OpCode::True:
      Push( Value{ true } );
      break;
    case OpCode::False:
      Push( Value{ false } );
      break;
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
    case OpCode::Return:
      std::cout << Pop() << '\n';
      return true; // return Pop() TODO
    }
  }
}

void VirtualMachine::Push( Value value )
{
  stack_.push_back( value );
}

Value VirtualMachine::Pop()
{
  assert( !stack_.empty() );
  Value top = stack_.back();
  stack_.pop_back();
  return top;
}

Value VirtualMachine::Peek() const
{
  assert( !stack_.empty() );
  return stack_.back();
}

uint8_t VirtualMachine::ReadByte() // private TODO ReadCode, NextByte ?
{
  return *ip_++;
}

///////////////////////////////////////////////////////////////////////////////
