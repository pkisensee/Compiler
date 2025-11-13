///////////////////////////////////////////////////////////////////////////////
//
//  ByteCodeBlock.h
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
#include <vector>

#include "Value.h"

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
  Return,
  Max
};

constexpr size_t kMaxOpCodes = static_cast<size_t>( OpCode::Max );
constexpr frozen::unordered_map< OpCode, std::string_view, kMaxOpCodes >
kOpCodeNames =
{
  { OpCode::Constant,     "Constant" },
  { OpCode::True,         "True" },
  { OpCode::False,        "False" },
  { OpCode::Empty,        "Empty" },
  { OpCode::Pop,          "Pop" },
  { OpCode::GetLocal,     "GetLocal" },
  { OpCode::GetGlobal,    "GetGlobal" },
  { OpCode::SetLocal,     "SetLocal" },
  { OpCode::DefineGlobal, "DefineGlobal" },
  { OpCode::SetGlobal,    "SetGlobal" },
  { OpCode::GetUpvalue,   "GetUpvalue" },
  { OpCode::SetUpvalue,   "SetUpvalue" },
  { OpCode::IsEqual,      "IsEqual" },
  { OpCode::Greater,      "Greater" },
  { OpCode::Less,         "Less" },
  { OpCode::Add,          "Add" },
  { OpCode::Subtract,     "Subtract" },
  { OpCode::Multiply,     "Multiply" },
  { OpCode::Divide,       "Divide" },
  { OpCode::Modulus,      "Modulus" },
  { OpCode::Negate,       "Negate" },
  { OpCode::Not,          "Not" },
  { OpCode::Print,        "Print" },
  { OpCode::Jump,         "Jump" },
  { OpCode::JumpIfFalse,  "JumpIfFalse" },
  { OpCode::Loop,         "Loop" },
  { OpCode::Call,         "Call" },
  { OpCode::Closure,      "Closure" },
  { OpCode::Return,       "Return" }
};

class ByteCodeBlock
{
public:
  using LineCount = uint16_t;

  ByteCodeBlock() = default;

  // Disable copy/move
  ByteCodeBlock( const ByteCodeBlock& ) = delete;
  ByteCodeBlock& operator=( const ByteCodeBlock& ) = delete;
  ByteCodeBlock( ByteCodeBlock&& ) = delete;
  ByteCodeBlock& operator=( ByteCodeBlock&& ) = delete;

  const uint8_t* GetEntryPoint() const
  {
    return byteCode_.data();
  }

  uint8_t* GetEntryPoint()
  {
    return byteCode_.data();
  }

  uint32_t GetCodeByteCount() const
  {
    auto size = byteCode_.size();
    assert( size <= std::numeric_limits<uint32_t>::max() );
    return static_cast<uint32_t>( size );
  }

  void Append( OpCode, LineCount line );
  void Append( uint8_t, LineCount line );
  size_t GetCurrOffset() const;
  void Free();
  uint8_t AddConstant( const Value& );
  const Value& GetConstant( uint8_t ) const;

  void Disassemble( std::string_view ) const;
  uint32_t DisassembleInstruction( uint32_t offset, const Value*, const std::string_view* ) const;
  void OutputOffset( uint32_t offset ) const;
  uint32_t OutputSimpleInstruction( std::string_view, uint32_t offset ) const;
  uint32_t OutputConstantInstruction( std::string_view, uint32_t offset ) const;
  uint32_t OutputLocalInstruction( std::string_view, uint32_t offset, const Value*, const std::string_view* ) const;
  uint32_t OutputCallInstruction( std::string_view, uint32_t offset ) const;
  uint32_t OutputClosureInstruction( std::string_view, uint32_t offset ) const;
  uint32_t OutputJumpInstruction( std::string_view, uint32_t offset, int32_t sign ) const;

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
  std::vector<uint8_t> byteCode_;
  std::vector<Value> constants_; // TODO optimize; compiler adds a global variable's name to the constant table every time an identifier is encountered (see https://craftinginterpreters.com/global-variables.html)
  std::vector<LineCount> lines_;

};

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
