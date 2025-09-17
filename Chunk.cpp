///////////////////////////////////////////////////////////////////////////////
//
//  Chunk.cpp
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

#include <cstdlib>
#include <iostream>
#include <limits>

#include "Chunk.h"
#include "CompilerError.h"

using namespace PKIsensee;

void Chunk::Append( OpCode opCode, LineCount line )
{
  Append( std::to_underlying(opCode), line );
}

void Chunk::Append( uint8_t value, LineCount line ) // writeChunk
{
  byteCode_.push_back( value );
  lines_.push_back( line );
}

uint32_t Chunk::GetCurrOffset() const
{
  return static_cast<uint32_t>( byteCode_.size() ); // TODO fix cast
}

void Chunk::Free()
{
  byteCode_.clear();
  constants_.clear();
  lines_.clear();
}

uint8_t Chunk::AddConstant( Value constant ) // TODO const Value& ? Is it more efficient?
{
  constants_.push_back( constant );
  if( constants_.size() >= std::numeric_limits<uint8_t>::max() )
    throw CompilerError( "Exceeded maximum number of constants" );
  return static_cast<uint8_t>( constants_.size() - 1 );
}

Value Chunk::GetConstant( uint8_t index ) const
{
  return constants_[ index ];
}

void Chunk::Disassemble( [[maybe_unused]] std::string_view name ) const
{
#if defined(DEBUG_TRACE_EXECUTION)
  std::string_view output = name.empty() ? "global scope" : name;
  std::cout << "  == " << output << " ==\n";
  // std::cout << "Byte Line Operation         Idx Value\n";
  for( uint32_t offset = 0u; offset < byteCode_.size(); )
    offset = DisassembleInstruction( offset, nullptr, nullptr );
#endif
}

uint32_t Chunk::DisassembleInstruction( uint32_t offset, const Value* slots, const std::string_view* names ) const
{
#if defined(DEBUG_TRACE_EXECUTION)
  assert( offset < byteCode_.size() );
  OutputOffset( offset );
  uint8_t byte = byteCode_[ offset ];
  OpCode opCode = static_cast<OpCode>( byte );
  switch( opCode )
  {
  // TODO frozen with opcode names
  case OpCode::Constant:      return OutputConstantInstruction( "Constant", offset );

  case OpCode::True:          return OutputSimpleInstruction( "True", offset );
  case OpCode::False:         return OutputSimpleInstruction( "False", offset );
  case OpCode::Empty:         return OutputSimpleInstruction( "Empty", offset );
  case OpCode::Pop:           return OutputSimpleInstruction( "Pop", offset );

  case OpCode::GetLocal:      return OutputLocalInstruction( "GetLocal", offset, slots, names );
  case OpCode::SetLocal:      return OutputLocalInstruction( "SetLocal", offset, slots, names );

  case OpCode::GetGlobal:     return OutputConstantInstruction( "GetGlobal", offset );
  case OpCode::DefineGlobal:  return OutputConstantInstruction( "DefineGlobal", offset );
  case OpCode::SetGlobal:     return OutputConstantInstruction( "SetGlobal", offset );

  case OpCode::GetUpvalue:    return OutputLocalInstruction( "GetUpvalue", offset, slots, names );
  case OpCode::SetUpvalue:    return OutputLocalInstruction( "SetUpvalue", offset, slots, names );

  case OpCode::IsEqual:       return OutputSimpleInstruction( "IsEqual", offset );
  case OpCode::Greater:       return OutputSimpleInstruction( "Greater", offset );
  case OpCode::Less:          return OutputSimpleInstruction( "Less", offset );
  case OpCode::Add:           return OutputSimpleInstruction( "Add", offset );
  case OpCode::Subtract:      return OutputSimpleInstruction( "Subtract", offset );
  case OpCode::Multiply:      return OutputSimpleInstruction( "Multiply", offset );
  case OpCode::Divide:        return OutputSimpleInstruction( "Divide", offset );
  case OpCode::Modulus:       return OutputSimpleInstruction( "Modulus", offset );
  case OpCode::Negate:        return OutputSimpleInstruction( "Negate", offset );
  case OpCode::Not:           return OutputSimpleInstruction( "Not", offset );
  case OpCode::Print:         return OutputSimpleInstruction( "Print", offset );

  case OpCode::Jump:          return OutputJumpInstruction( "Jump", offset, 1 );
  case OpCode::JumpIfFalse:   return OutputJumpInstruction( "JumpIfFalse", offset, 1 );
  case OpCode::Loop:          return OutputJumpInstruction( "Loop", offset, -1 );

  case OpCode::Call:          return OutputCallInstruction( offset );
  case OpCode::Closure:       return OutputClosureInstruction( offset );

  case OpCode::Return:        return OutputSimpleInstruction( "Return", offset );

  default:
    std::cout << std::format( "Unknown opcode {}\n", std::to_underlying( opCode ) );
    return offset + 1; // TODO store the sizes in an array somewhere
  }
#else
  ( void )offset;
  ( void )slots;
  ( void )names;
#endif
}

void Chunk::OutputOffset( uint32_t offset ) const
{
  std::cout << std::format( "{:04d} ", offset );
}

uint32_t Chunk::OutputSimpleInstruction( std::string_view name, uint32_t offset ) const
{
  OutputInstructionDetails( name );
  return offset + 1;
}

uint32_t Chunk::OutputConstantInstruction( std::string_view name, uint32_t offset ) const
{
  uint8_t constantIndex = byteCode_[ offset + 1 ];
  auto value = constants_[ constantIndex ];
  OutputInstructionDetails( name, ' ', value );
  return offset + 2;
}

uint32_t Chunk::OutputLocalInstruction( std::string_view opName, uint32_t offset, 
                                        const Value* slots, const std::string_view* names ) const
{
  uint8_t localIndex = byteCode_[ offset + 1 ];
  if( slots && names )
  {
    std::string_view localName = names[localIndex];
    Value localValue = slots[localIndex];
    OutputInstructionDetails( opName, ' ', localName, '=', localValue);
  }
  else
    OutputInstructionDetails( opName, std::format( " [{}]", localIndex ) );
  return offset + 2;
}

uint32_t Chunk::OutputCallInstruction( uint32_t offset ) const
{
  uint8_t argCount = byteCode_[ offset + 1 ];
  OutputInstructionDetails( "Call", std::format( " args={}", argCount ) );
  return offset + 2;
}

uint32_t Chunk::OutputClosureInstruction( uint32_t offset ) const
{
  uint8_t constant = byteCode_[ ++offset ];
  Value value = GetConstant( constant );
  Closure closure = value.GetClosure();
  Function function = closure.GetFunction();
  OutputInstructionDetails( "Closure", std::format( " [{}]", constant ) );

  // Upvalues
  for( uint8_t i = 0; i < function.GetUpvalueCount(); ++i )
  {
    auto isLocal = byteCode_[ ++offset ];
    auto index = byteCode_[ ++offset ];
    OutputInstructionDetails( "     Capture", 
      std::format( " [{}] {}", index, isLocal ? "local" : "upvalue" ) );
  }

  return ++offset;
}

uint32_t Chunk::OutputJumpInstruction( std::string_view name, uint32_t offset, int32_t sign ) const
{
  const uint8_t* code = byteCode_.data() + offset;
  ++code; // skip opcode
  uint8_t jumpHi = *code++;
  uint8_t jumpLo = *code;
  uint16_t jumpBytes = static_cast<uint16_t>(( jumpHi << 8 ) | jumpLo );
  uint32_t jumpLocation = offset + 3 + ( sign * jumpBytes );
  OutputInstructionDetails( name, ' ', jumpLocation );
  return offset + 3;
}

///////////////////////////////////////////////////////////////////////////////