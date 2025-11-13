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

#include <cstdlib>
#include <iostream>
#include <limits>

#include "ByteCodeBlock.h"
#include "CompilerError.h"

using namespace PKIsensee;

void ByteCodeBlock::Append( OpCode opCode, LineCount line )
{
  Append( std::to_underlying(opCode), line );
}

void ByteCodeBlock::Append( uint8_t value, LineCount line ) // writeChunk
{
  byteCode_.push_back( value );
  lines_.push_back( line );
}

size_t ByteCodeBlock::GetCurrOffset() const
{
  return byteCode_.size();
}

void ByteCodeBlock::Free()
{
  byteCode_.clear();
  constants_.clear();
  lines_.clear();
}

uint8_t ByteCodeBlock::AddConstant( const Value& constant )
{
  constants_.push_back( constant );
  if( constants_.size() >= std::numeric_limits<uint8_t>::max() )
    throw CompilerError( "Exceeded maximum number of constants" );
  return static_cast<uint8_t>( constants_.size() - 1 );
}

Value ByteCodeBlock::GetConstant( uint8_t index ) const
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

uint32_t ByteCodeBlock::DisassembleInstruction( uint32_t offset, const Value* slots, const std::string_view* names ) const
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
    return offset + 1; // TODO store the sizes in an array somewhere
  }
#else
  [[maybe_unused]] offset;
  [[maybe_unused]] slots;
  [[maybe_unused]] names;
#endif
}

void ByteCodeBlock::OutputOffset( uint32_t offset ) const
{
  std::cout << std::format( "{:04d} ", offset );
}

uint32_t ByteCodeBlock::OutputSimpleInstruction( std::string_view name, uint32_t offset ) const
{
  OutputInstructionDetails( name );
  return offset + 1;
}

uint32_t ByteCodeBlock::OutputConstantInstruction( std::string_view name, uint32_t offset ) const
{
  uint8_t constantIndex = byteCode_[ offset + 1 ];
  auto value = constants_[ constantIndex ];
  OutputInstructionDetails( name, ' ', value );
  return offset + 2;
}

uint32_t ByteCodeBlock::OutputLocalInstruction( std::string_view opName, uint32_t offset,
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

uint32_t ByteCodeBlock::OutputCallInstruction( std::string_view opName, uint32_t offset ) const
{
  uint8_t argCount = byteCode_[ offset + 1 ];
  OutputInstructionDetails( opName, std::format( " args={}", argCount ) );
  return offset + 2;
}

uint32_t ByteCodeBlock::OutputClosureInstruction( std::string_view opName, uint32_t offset ) const
{
  uint8_t constant = byteCode_[ ++offset ];
  Value value = GetConstant( constant );
  const Closure& closure = value.GetClosure();
  const Function& function = closure.GetFunction();
  OutputInstructionDetails( opName, std::format( " [{}]", constant ) );

  // Upvalues
  for( uint32_t i = 0; i < function.GetUpvalueCount(); ++i )
  {
    auto isLocal = byteCode_[ ++offset ];
    auto index = byteCode_[ ++offset ];
    OutputInstructionDetails( "     Capture", 
      std::format( " [{}] {}", index, isLocal ? "local" : "upvalue" ) );
  }

  return ++offset;
}

uint32_t ByteCodeBlock::OutputJumpInstruction( std::string_view name, uint32_t offset, int32_t sign ) const
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