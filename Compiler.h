///////////////////////////////////////////////////////////////////////////////
//
//  Compiler.h
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
#define DEBUG_PRINT_CODE 1
#include <initializer_list>
#include <string_view>

#include "Lexer.h"
#include "Value.h"

namespace PKIsensee
{

enum class Precedence // lowest to highest
{
  None,
  Assignment, // =
  Or,         // or
  And,        // and
  Equality,   // == !=
  Comparison, // < > <= >-
  Add,        // + -
  Mult,       // * /
  Unary,      // ! -
  Call,       // . ()
  Primary
};

inline Precedence& operator++( Precedence& precedence )
{
  return precedence = static_cast<Precedence>( static_cast<int>( precedence ) + 1 );
}

class Chunk;

class Compiler
{
public:
  Compiler() = default;

  // Disable copy/move
  Compiler( const Compiler& ) = delete;
  Compiler& operator=( const Compiler& ) = delete;
  Compiler( Compiler&& ) = delete;
  Compiler& operator=( Compiler&& ) = delete;

  bool Compile( std::string_view, Chunk* );

  typedef void ( Compiler::*ParseFn )( );
  class ParseRule
  {
  public:
    ParseFn prefix_ = nullptr;
    ParseFn infix_ = nullptr;
    Precedence precedence_ = Precedence::None;
  };

public:
  void Grouping();
  void Number();
  void Unary();
  void Binary();
  void Literal();

private:

  Chunk* GetCurrentChunk()
  {
    return compilingChunk_;
  }

  void Advance();
  void Expression();
  void Consume( TokenType, std::string_view );

  void ParsePrecedence( Precedence );
  ParseFn GetPrefixFn() const;
  ParseFn GetInfixFn() const;
  const ParseRule& GetRule( TokenType ) const;

  void EmitConstant( Value );
  uint8_t MakeConstant( Value );

  void EmitByte( OpCode );
  void EmitByte( uint8_t );
  void EmitBytes( OpCode, OpCode );
  void EmitBytes( OpCode, uint8_t );

private:

  Lexer lexer_;
  TokenList::const_iterator prevToken_;
  TokenList::const_iterator currToken_;
  Chunk* compilingChunk_ = nullptr;

}; // class Compiler

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
