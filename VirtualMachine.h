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
#include "Value.h"

namespace PKIsensee
{

using InterpretResult = bool; // TODO may want to track specific errors in future
class Chunk;

class VirtualMachine
{
public:
  VirtualMachine() = default;

  void Reset();
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

  template< typename UnaryOp >
  void LogicalUnaryOp( UnaryOp logicalUnaryOp )
  {
    assert( !stack_.empty() );
    stack_.back() = Value{ logicalUnaryOp( stack_.back() ) };
  }

  template< typename BinOp >
  void BinaryOp( BinOp binOp )
  {
    Value rhs = Pop();
    Value lhs = Pop();
    Push( binOp( lhs, rhs ) );
  }

  template< typename BinOp >
  void LogicalBinaryOp( BinOp binOp )
  {
    Value rhs = Pop();
    Value lhs = Pop();
    Push( Value{ binOp( lhs, rhs ) } );
  }

  uint8_t ReadByte();
  InterpretResult Run();
  void Push( Value );
  Value Pop();
  Value Peek() const;

private:
  Compiler compiler_;
  const Chunk* chunk_ = nullptr;
  const uint8_t* ip_ = nullptr; // instruction pointer
  std::vector<Value> stack_;

};

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
