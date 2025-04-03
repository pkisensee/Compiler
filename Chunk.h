///////////////////////////////////////////////////////////////////////////////
//
//  Chunk.h
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
#include <cstdint>
#include <string_view>

#include "DynArray.h"
#include "Value.h"

// TODO Chunk -> ByteCodeBlock or ByteCode or ?

namespace PKIsensee
{

enum class OpCode : uint8_t
{
  Constant,
  True,
  False,
  Empty,
  Pop,
  GetLocal,
  GetGlobal,
  SetLocal,
  DefineGlobal,
  SetGlobal,
  GetUpvalue,
  SetUpvalue,
  IsEqual,
  Greater,
  Less,
  Add,
  Subtract,
  Multiply,
  Divide,
  Modulus,
  Negate,
  Not,
  Print,
  Jump,
  JumpIfFalse,
  Loop,
  Call,
  Closure,
  Return
};

class Chunk
{
public:
  using LineCount = uint16_t;

  Chunk() = default;

  const uint8_t* GetCode() const // TODO GetEntryPoint()
  {
    return byteCode_.GetPtr();
  }

  uint8_t* GetCode() // TODO GetEntryPoint()
  {
    return byteCode_.GetPtr();
  }

  uint32_t GetCodeByteCount() const
  {
    return byteCode_.GetCount();
  }

  void Append( OpCode, LineCount line );
  void Append( uint8_t, LineCount line );
  uint32_t GetCurrOffset() const;
  void Free();
  uint8_t AddConstant( Value );
  Value GetConstant( uint8_t ) const;

  void Disassemble( std::string_view ) const;
  uint32_t DisassembleInstruction( uint32_t offset, const Value*, const std::string_view* ) const;
  void OutputOffset( uint32_t offset ) const;
  uint32_t OutputSimpleInstruction( std::string_view, uint32_t ) const;
  uint32_t OutputConstantInstruction( std::string_view, uint32_t offset ) const;
  uint32_t OutputLocalInstruction( std::string_view, uint32_t offset, const Value*, const std::string_view* ) const;
  uint32_t OutputCallInstruction( uint32_t offset ) const;
  uint32_t OutputClosureInstruction( uint32_t offset ) const;
  uint32_t OutputJumpInstruction( std::string_view, uint32_t offset, int32_t sign ) const;

  // Disable copy/move
  Chunk( const Chunk& ) = delete;
  Chunk& operator=( const Chunk& ) = delete;
  Chunk( Chunk&& ) = delete;
  Chunk& operator=( Chunk&& ) = delete;

private:
  template <typename Arg, typename... Args>
  void OutputInstructionDetails( [[maybe_unused]] Arg&& arg, [[maybe_unused]] Args&&... args ) const
  {
#if defined(DEBUG_TRACE_EXECUTION)
    std::cout << arg;
    ( ( std::cout << std::forward<Args>( args ) ), ... );
    std::cout << '\n';
#endif
  }

private:
  DynArray<uint8_t> byteCode_; // std::vector?
  DynArray<Value> constants_; // TODO optimize; compiler adds a global variable's name to the constant table every time an identifier is encountered (see https://craftinginterpreters.com/global-variables.html)
  DynArray<LineCount> lines_;

};

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
