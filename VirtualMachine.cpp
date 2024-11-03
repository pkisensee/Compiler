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
    uint8_t instruction = ReadByte();
    OpCode opCode = static_cast<OpCode>( instruction );
    switch( opCode )
    {
    case OpCode::Constant: 
    {
      uint8_t index = ReadByte();
      uint64_t constant = chunk_->GetConstant( index );
      std::cout << constant << '\n';
      break;
    }
    case OpCode::Return: return;
    }
  }
}

uint8_t VirtualMachine::ReadByte() // private TODO ReadCode, NextByte ?
{
  return *ip_++;
}

///////////////////////////////////////////////////////////////////////////////
