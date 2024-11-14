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

#include "Compiler.h"

namespace PKIsensee
{

using InterpretResult = bool; // TODO may want to track specific errors in future
class Chunk;

class VirtualMachine
{
public:
  VirtualMachine() = default;

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
    stack_.back() = unaryOp( stack_.back() );
  }

  template< typename BinOp >
  void BinaryOp( BinOp binOp )
  {
    int64_t rhs = Pop();
    int64_t lhs = Pop();
    Push( binOp( lhs, rhs ) );
  }

  uint8_t ReadByte();
  InterpretResult Run();
  void Push( int64_t );
  int64_t Pop();
  Value Peek() const;

private:
  Compiler compiler_;
  const Chunk* chunk_ = nullptr;
  const uint8_t* ip_ = nullptr; // instruction pointer
  std::vector<int64_t> stack_;  // uint64_t -> Value TODO

};

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
