///////////////////////////////////////////////////////////////////////////////
//
//  ByteCodeBlock.cpp
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
#include <cstdlib>
#include <iostream>
#include <limits>
#include <ranges>

#include "ByteCodeBlock.h"
#include "CompilerError.h"

using namespace PKIsensee;

static constexpr uint32_t kSimpleInstructionSize    = 1;
static constexpr uint32_t kConstantInstructionSize  = 2;
static constexpr uint32_t kLocalInstructionSize     = 2;
static constexpr uint32_t kCallInstructionSize      = 2;
static constexpr uint32_t kJumpInstructionSize      = 3;

#pragma warning(push)
#pragma warning(disable: 5264) // variable not used
[[maybe_unused]] static constexpr uint32_t kClosureInstructionSize = 2; // not including upvalues
[[maybe_unused]] static constexpr uint32_t kUpvalueInstructionSize = 2;
#pragma warning(pop)

void ByteCodeBlock::Append( OpCode opCode, LineCount line )
{
  Append( std::to_underlying(opCode), line );
}

void ByteCodeBlock::Append( uint8_t value, LineCount line )
{
  byteCode_.push_back( value );
  lines_.push_back( line );
}

size_t ByteCodeBlock::GetCurrOffset() const
{
  return byteCode_.size();
}

// Returns the index of the new constant
uint8_t ByteCodeBlock::AddConstant( const Value& constant )
{
  // If constant already recorded, return index
  auto it = std::ranges::find( constants_, constant );
  if ( it != constants_.end() )
    return static_cast<uint8_t>( std::distance( constants_.begin(), it) );

  // New constant, add
  constants_.push_back( constant );
  if( constants_.size() >= std::numeric_limits<uint8_t>::max() )
    throw CompilerError( "Exceeded maximum number of constants" );
  return static_cast<uint8_t>( constants_.size() - 1 );
}

const Value& ByteCodeBlock::GetConstant( uint8_t index ) const
{
  return constants_[ index ];
}

void ByteCodeBlock::Disassemble( [[maybe_unused]] std::string_view name ) const
{
#if defined(DEBUG_TRACE_EXECUTION)
  std::string_view output = name.empty() ? "global scope" : name;
  std::cout << "  == " << output << " ==\n";
  // std::cout << "Byte Line Operation         Idx Value\n";
  for( uint32_t offset = 0u; offset < byteCode_.size(); )
    offset = DisassembleInstruction( offset, nullptr, nullptr );
#endif
}

uint32_t ByteCodeBlock::DisassembleInstruction( [[maybe_unused]] uint32_t offset, 
  [[maybe_unused]] const Value* slots, [[maybe_unused]] const std::string_view* names ) const
{
#if defined(DEBUG_TRACE_EXECUTION)
  assert( offset < byteCode_.size() );
  OutputOffset( offset );
  uint8_t byte = byteCode_[ offset ];
  OpCode opCode = static_cast<OpCode>( byte );
  std::string_view opCodeName = kOpCodeNames.at( opCode );
  switch( opCode )
  {
  case OpCode::Constant:
  case OpCode::GetGlobal:
  case OpCode::DefineGlobal:
  case OpCode::SetGlobal:
    return OutputConstantInstruction( opCodeName, offset );

  case OpCode::GetLocal:      
  case OpCode::SetLocal:
  case OpCode::GetUpvalue:    
  case OpCode::SetUpvalue:
    return OutputLocalInstruction( opCodeName, offset, slots, names );

  case OpCode::True:
  case OpCode::False:
  case OpCode::Empty:
  case OpCode::Pop:
  case OpCode::IsEqual:
  case OpCode::Greater: 
  case OpCode::Less:    
  case OpCode::Add:     
  case OpCode::Subtract:
  case OpCode::Multiply:
  case OpCode::Divide:  
  case OpCode::Modulus: 
  case OpCode::Negate:  
  case OpCode::Not:     
  case OpCode::Print:
  case OpCode::Return:
    return OutputSimpleInstruction( opCodeName, offset );

  case OpCode::Jump:
  case OpCode::JumpIfFalse:
  case OpCode::Loop:
    return OutputJumpInstruction( opCodeName, offset, -1 );

  case OpCode::Call:
    return OutputCallInstruction( opCodeName, offset );

  case OpCode::Closure:
    return OutputClosureInstruction( opCodeName, offset );

  case OpCode::Max:
  default:
    std::cout << std::format( "Unknown opcode {}\n", std::to_underlying( opCode ) );
    return offset + 1;
  }
#endif
}

void ByteCodeBlock::OutputOffset( uint32_t offset ) const
{
  std::cout << std::format( "{:04d} ", offset );
}

uint32_t ByteCodeBlock::OutputSimpleInstruction( std::string_view name, uint32_t offset ) const
{
  OutputInstructionDetails( name );
  return offset + kSimpleInstructionSize;
}

uint32_t ByteCodeBlock::OutputConstantInstruction( std::string_view name, uint32_t offset ) const
{
  uint8_t constantIndex = byteCode_[ offset + 1 ];
  const Value& value = constants_[ constantIndex ];
  OutputInstructionDetails( name, ' ', value );
  return offset + kConstantInstructionSize;
}

uint32_t ByteCodeBlock::OutputLocalInstruction( std::string_view opName, uint32_t offset,
                                                const Value* slots, const std::string_view* names ) const
{
  uint8_t localIndex = byteCode_[ offset + 1 ];
  if( slots && names )
  {
    std::string_view localName = names[localIndex];
    const Value& localValue = slots[localIndex];
    OutputInstructionDetails( opName, ' ', localName, '=', localValue );
  }
  else
    OutputInstructionDetails( opName, std::format( " [{}]", localIndex ) );
  return offset + kLocalInstructionSize;
}

uint32_t ByteCodeBlock::OutputCallInstruction( std::string_view opName, uint32_t offset ) const
{
  uint8_t argCount = byteCode_[ offset + 1 ];
  OutputInstructionDetails( opName, std::format( " args={}", argCount ) );
  return offset + kCallInstructionSize;
}

uint32_t ByteCodeBlock::OutputClosureInstruction( std::string_view opName, uint32_t offset ) const
{
  [[maybe_unused]] uint32_t startOffset = offset;
  uint8_t constant = byteCode_[ ++offset ];
  OutputInstructionDetails( opName, std::format( " [{}]", constant ) );

  const Value& value = GetConstant( constant );
  const Closure& closure = value.GetClosure();
  const Function& function = closure.GetFunction();

  // Upvalues
  for( uint32_t i = 0; i < function.GetUpvalueCount(); ++i )
  {
    auto isLocal = byteCode_[ ++offset ];
    auto index = byteCode_[ ++offset ];
    OutputInstructionDetails( "     Capture", 
      std::format( " [{}] {}", index, isLocal ? "local" : "upvalue" ) );
  }

  ++offset;
  assert( startOffset + kClosureInstructionSize + 
          ( kUpvalueInstructionSize * function.GetUpvalueCount() ) == offset );
  return offset;
}

uint32_t ByteCodeBlock::OutputJumpInstruction( std::string_view name, uint32_t offset, int32_t sign ) const
{
  const uint8_t* code = byteCode_.data() + offset;
  ++code; // skip opcode
  uint8_t jumpHi = *code++;
  uint8_t jumpLo = *code;
  uint16_t jumpBytes = static_cast<uint16_t>(( jumpHi << 8 ) | jumpLo );
  uint32_t jumpLocation = offset + kJumpInstructionSize + ( sign * jumpBytes );
  OutputInstructionDetails( name, ' ', jumpLocation );
  return offset + kJumpInstructionSize;
}

///////////////////////////////////////////////////////////////////////////////