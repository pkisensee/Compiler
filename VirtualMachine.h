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
#include <vector>

namespace PKIsensee
{

class Chunk;

class VirtualMachine
{
public:
  VirtualMachine() = default;

  void Interpret( const Chunk* );

  // Disable copy/move
  VirtualMachine( const VirtualMachine& ) = delete;
  VirtualMachine& operator=( const VirtualMachine& ) = delete;
  VirtualMachine( VirtualMachine&& ) = delete;
  VirtualMachine& operator=( VirtualMachine&& ) = delete;

private:

  template< typename BinOp >
  void BinaryOp( BinOp binOp )
  {
    int64_t rhs = Pop();
    int64_t lhs = Pop();
    Push( binOp( lhs, rhs ) );
  }

  uint8_t ReadByte();
  void Run();
  void Push( int64_t );
  int64_t Pop();

private:
  const Chunk* chunk_ = nullptr;
  const uint8_t* ip_ = nullptr; // instruction pointer
  std::vector<int64_t> stack_;  // uint64_t -> Value TODO

};

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
