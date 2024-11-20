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

void Chunk::Free()
{
  byteCode_.Free();
  constants_.Free();
  lines_.Free();
}

uint8_t Chunk::AddConstant( Value constant )
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
  std::cout << "== " << name << " ==\n";
  std::cout << "Byte Line Operation         Idx Value\n";
  for( uint32_t offset = 0u; offset < byteCode_.GetCount(); )
    offset = DisassembleInstruction( offset );
}

uint32_t Chunk::DisassembleInstruction( uint32_t offset ) const
{
  assert( offset < lines_.GetCount() );
  std::cout << std::format( "{:04d} ", offset );
  if( offset > 0 && lines_.Get( offset ) == lines_.Get( offset - 1 ) )
    std::cout << "   | ";
  else
    std::cout << std::format( "{:04d} ", lines_.Get( offset ) );
  uint8_t byte = byteCode_.Get( offset );
  OpCode opCode = static_cast<OpCode>( byte );
  switch( opCode )
  {
  // TODO frozen with opcode names
  case OpCode::Constant: return OutputConstantInstruction( "Constant", offset );
  case OpCode::True:     return OutputSimpleInstruction( "True", offset );
  case OpCode::False:    return OutputSimpleInstruction( "False", offset );
  case OpCode::IsEqual:  return OutputSimpleInstruction( "IsEqual", offset );
  case OpCode::Greater:  return OutputSimpleInstruction( "Greater", offset );
  case OpCode::Less:     return OutputSimpleInstruction( "Less", offset );
  case OpCode::Add:      return OutputSimpleInstruction( "Add", offset );
  case OpCode::Subtract: return OutputSimpleInstruction( "Subtract", offset );
  case OpCode::Multiply: return OutputSimpleInstruction( "Multiply", offset );
  case OpCode::Divide:   return OutputSimpleInstruction( "Divide", offset );
  case OpCode::Negate:   return OutputSimpleInstruction( "Negate", offset );
  case OpCode::Not:      return OutputSimpleInstruction( "Not", offset );
  case OpCode::Return:   return OutputSimpleInstruction( "Return", offset );
  default: 
    std::cout << std::format( "Unknown opcode {}\n", std::to_underlying( opCode ) );
    return offset + 1; // TODO store the sizes in an array somewhere
  }
}

uint32_t Chunk::OutputConstantInstruction( std::string_view name, uint32_t offset ) const
{
  uint8_t constantIndex = byteCode_.Get(offset + 1);
  auto value = constants_.Get( constantIndex );
  std::cout << std::format( "{:<16} {:4d} {}\n", name, constantIndex, value );
  return offset + 2;
}

///////////////////////////////////////////////////////////////////////////////
