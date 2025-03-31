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

namespace { // anonymous

uint32_t OutputSimpleInstruction( std::string_view name, uint32_t offset )
{
  std::cout << name << '\n';
  return offset + 1;
}

} // anonymous

void Chunk::Append( OpCode opCode, LineCount line )
{
  Append( std::to_underlying(opCode), line );
}

void Chunk::Append( uint8_t value, LineCount line ) // writeChunk
{
  byteCode_.Append( value );
  lines_.Append( line );
}

uint32_t Chunk::GetCurrOffset() const
{
  return byteCode_.GetCount();
}

void Chunk::Free()
{
  byteCode_.Free();
  constants_.Free();
  lines_.Free();
}

uint8_t Chunk::AddConstant( Value constant ) // TODO const Value& ? Is it more efficient?
{
  constants_.Append( constant );
  if( constants_.GetCount() >= std::numeric_limits<uint8_t>::max() )
    throw CompilerError( "Exceeded maximum number of constants" );
  return static_cast<uint8_t>( constants_.GetCount() - 1 );
}

Value Chunk::GetConstant( uint8_t index ) const
{
  return constants_.Get( index );
}

void Chunk::Disassemble( std::string_view name ) const
{
  // TODO is this used anywhere?
  std::cout << "== " << name << " ==\n";
  std::cout << "Byte Line Operation         Idx Value\n";
  for( uint32_t offset = 0u; offset < byteCode_.GetCount(); )
    offset = DisassembleInstruction( offset, nullptr, nullptr );
}

uint32_t Chunk::DisassembleInstruction( uint32_t offset, const Value* slots, const std::string_view* names ) const
{
  // TODO do we need the return value anymore?
  assert( offset < byteCode_.GetCount() );
  std::cout << std::format( "{:04d} ", offset );
  //if( offset > 0 && lines_.Get( offset ) == lines_.Get( offset - 1 ) )
  //  std::cout << "   | ";
  //else
  //  std::cout << std::format( "{:04d} ", lines_.Get( offset ) );
  uint8_t byte = byteCode_.Get( offset );
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

  case OpCode::IsEqual:       return OutputSimpleInstruction( "IsEqual", offset );
  case OpCode::Greater:       return OutputSimpleInstruction( "Greater", offset );
  case OpCode::Less:          return OutputSimpleInstruction( "Less", offset );
  case OpCode::Add:           return OutputSimpleInstruction( "Add", offset );
  case OpCode::Subtract:      return OutputSimpleInstruction( "Subtract", offset );
  case OpCode::Multiply:      return OutputSimpleInstruction( "Multiply", offset );
  case OpCode::Divide:        return OutputSimpleInstruction( "Divide", offset );
  case OpCode::Negate:        return OutputSimpleInstruction( "Negate", offset );
  case OpCode::Not:           return OutputSimpleInstruction( "Not", offset );
  case OpCode::Print:         return OutputSimpleInstruction( "Print", offset );

  case OpCode::Jump:          return OutputJumpInstruction( "Jump", offset, 1 );
  case OpCode::JumpIfFalse:   return OutputJumpInstruction( "JumpIfFalse", offset, 1 );
  case OpCode::Loop:          return OutputJumpInstruction( "Loop", offset, -1 );

  case OpCode::Call:          return OutputCallInstruction( "Call", offset );

  case OpCode::Return:        return OutputSimpleInstruction( "Return", offset );
  default:
    std::cout << std::format( "Unknown opcode {}\n", std::to_underlying( opCode ) );
    return offset + 1; // TODO store the sizes in an array somewhere
  }
}

uint32_t Chunk::OutputConstantInstruction( std::string_view name, uint32_t offset ) const
{
  uint8_t constantIndex = byteCode_.Get(offset + 1);
  auto value = constants_.Get( constantIndex );
  std::cout << std::format( "{}: {}\n", name, value );
  return offset + 2;
}

uint32_t Chunk::OutputByteInstruction( std::string_view name, uint32_t offset ) const
{
  uint8_t index = byteCode_.Get( offset + 1 );
  std::cout << std::format( "{}: [{}]\n", name, index );
  return offset + 2;
}

uint32_t Chunk::OutputLocalInstruction( std::string_view opName, uint32_t offset, 
                                        const Value* slots, const std::string_view* names ) const
{
  uint8_t localIndex = byteCode_.Get( offset + 1 );
  if( slots && names )
  {
    std::string_view localName = names[localIndex];
    Value localValue = slots[localIndex];
    std::cout << std::format( "{}: {}={}\n", opName, localName, localValue );
  }
  else
    std::cout << std::format( "{}: [{}]\n", opName, localIndex );
  return offset + 2;
}

uint32_t Chunk::OutputCallInstruction( std::string_view name, uint32_t offset ) const
{
  uint8_t argCount = byteCode_.Get( offset + 1 );
  std::cout << std::format( "{}: args={}\n", name, argCount );
  return offset + 2;
}

uint32_t Chunk::OutputJumpInstruction( std::string_view name, uint32_t offset, int32_t sign ) const
{
  const uint8_t* code = byteCode_.GetPtr() + offset;
  ++code; // skip opcode
  uint8_t jumpHi = *code++;
  uint8_t jumpLo = *code;
  uint16_t jumpBytes = static_cast<uint16_t>(( jumpHi << 8 ) | jumpLo );
  uint32_t jumpLocation = offset + 3 + ( sign * jumpBytes );
  std::cout << std::format( "{}: {}\n", name, jumpLocation );
  return offset + 3;
}

///////////////////////////////////////////////////////////////////////////////
