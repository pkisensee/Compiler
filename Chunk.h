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
  IsEqual,
  Greater,
  Less,
  Add,
  Subtract,
  Multiply,
  Divide,
  Negate,
  Not,
  Print,
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

  void Append( OpCode, LineCount line );
  void Append( uint8_t, LineCount line );
  void Free();
  uint8_t AddConstant( Value );
  Value GetConstant( uint8_t ) const;

  void Disassemble( std::string_view ) const;
  uint32_t DisassembleInstruction( uint32_t offset ) const;
  uint32_t OutputConstantInstruction( std::string_view, uint32_t offset ) const;

  // Disable copy/move
  Chunk( const Chunk& ) = delete;
  Chunk& operator=( const Chunk& ) = delete;
  Chunk( Chunk&& ) = delete;
  Chunk& operator=( Chunk&& ) = delete;

private:
  DynArray<uint8_t> byteCode_;
  DynArray<Value> constants_;
  DynArray<LineCount> lines_;

};

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
