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

#include "Chunk.h"
#include "VirtualMachine.h"

using namespace PKIsensee;

void VirtualMachine::Interpret( const Chunk* chunk )
{
  assert( chunk != nullptr );
  chunk_ = chunk;
  ip_ = chunk_->GetCode();
  Run();
}

void VirtualMachine::Run() // private
{
  for( ;; )
  {
#if defined(DEBUG_TRACE_EXECUTION)
    std::cout << "          ";
    for( uint64_t slot : stack_ )
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
      uint64_t constant = chunk_->GetConstant( index );
      Push( constant );
      break;
    }
    case OpCode::Return:
      std::cout << Pop() << '\n';
      return;
    }
  }
}

void VirtualMachine::Push( uint64_t value )
{
  stack_.push_back( value );
}

uint64_t VirtualMachine::Pop()
{
  assert( !stack_.empty() );
  uint64_t top = stack_.back();
  stack_.pop_back();
  return top;
}

uint8_t VirtualMachine::ReadByte() // private TODO ReadCode, NextByte ?
{
  return *ip_++;
}

///////////////////////////////////////////////////////////////////////////////
