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
    for( int64_t slot : stack_ )
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
      int64_t constant = chunk_->GetConstant( index );
      Push( constant );
      break;
    }
    case OpCode::Add:
      BinaryOp( std::plus<int64_t>() );
      break;
    case OpCode::Subtract:
      BinaryOp( std::minus<int64_t>() );
      break;
    case OpCode::Multiply:
      BinaryOp( std::multiplies<int64_t>() );
      break;
    case OpCode::Divide:
      BinaryOp( std::divides<int64_t>() );
      break;
    case OpCode::Negate:
      UnaryOp( std::negate<int64_t>() );
      // Push( -Pop() );
      break;
    case OpCode::Return:
      std::cout << Pop() << '\n';
      return true; // return Pop() TODO
    }
  }
}

void VirtualMachine::Push( int64_t value )
{
  stack_.push_back( value );
}

int64_t VirtualMachine::Pop()
{
  assert( !stack_.empty() );
  int64_t top = stack_.back();
  stack_.pop_back();
  return top;
}

uint8_t VirtualMachine::ReadByte() // private TODO ReadCode, NextByte ?
{
  return *ip_++;
}

///////////////////////////////////////////////////////////////////////////////
