///////////////////////////////////////////////////////////////////////////////
//
//  Compiler.cpp
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

#include <array>
#include <utility>
#include <vector>

#include "Chunk.h"
#include "Compiler.h"
#include "CompilerError.h"
#include "Lexer.h"
#include "Token.h"
#include "Value.h"

using namespace PKIsensee;

std::array<Compiler::ParseRule, static_cast<size_t>(TokenType::Last)> kParseRules =
{ {
  {nullptr,             nullptr,            Precedence::None}, // OpenBracket
  {nullptr,             nullptr,            Precedence::None}, // CloseBracket
  {nullptr,             nullptr,            Precedence::None}, // OpenBrace
  {nullptr,             nullptr,            Precedence::None}, // CloseBrace
  {&Compiler::Grouping, nullptr,            Precedence::None}, // OpenParen
  {nullptr,             nullptr,            Precedence::None}, // CloseParen
  {nullptr,             nullptr,            Precedence::None}, // LessThan
  {nullptr,             nullptr,            Precedence::None}, // GreaterThan
  {nullptr,             nullptr,            Precedence::None}, // EndStatement
  {nullptr,             nullptr,            Precedence::None}, // Assign
  {nullptr,             &Compiler::Binary,  Precedence::Add }, // Plus
  {&Compiler::Unary,    &Compiler::Binary,  Precedence::Add }, // Minus
  {nullptr,             &Compiler::Binary,  Precedence::Mult}, // Multiply
  {nullptr,             &Compiler::Binary,  Precedence::Mult}, // Divide
  {nullptr,             nullptr,            Precedence::None}, // Comma
  {nullptr,             nullptr,            Precedence::None}, // Dot
  {nullptr,             nullptr,            Precedence::None}, // IsEqual
  {nullptr,             nullptr,            Precedence::None}, // NotEqual
  {nullptr,             nullptr,            Precedence::None}, // LessThanEqual
  {nullptr,             nullptr,            Precedence::None}, // GreaterThanEqual
  {&Compiler::Number,   nullptr,            Precedence::None}, // Number
  {nullptr,             nullptr,            Precedence::None}, // Identifier
  {nullptr,             nullptr,            Precedence::None}, // String
  {nullptr,             nullptr,            Precedence::None}, // And
  {nullptr,             nullptr,            Precedence::None}, // Or
  {nullptr,             nullptr,            Precedence::None}, // Not
  {nullptr,             nullptr,            Precedence::None}, // If
  {nullptr,             nullptr,            Precedence::None}, // Else
  {nullptr,             nullptr,            Precedence::None}, // For
  {nullptr,             nullptr,            Precedence::None}, // While
  {nullptr,             nullptr,            Precedence::None}, // Return
  {nullptr,             nullptr,            Precedence::None}, // True
  {nullptr,             nullptr,            Precedence::None}, // False
  {nullptr,             nullptr,            Precedence::None}, // Print
  {nullptr,             nullptr,            Precedence::None}, // Str
  {nullptr,             nullptr,            Precedence::None}, // Int
  {nullptr,             nullptr,            Precedence::None}, // Char
  {nullptr,             nullptr,            Precedence::None}, // Bool
  {nullptr,             nullptr,            Precedence::None}, // Function
  {nullptr,             nullptr,            Precedence::None}, // Invalid
  {nullptr,             nullptr,            Precedence::None}, // EndOfFile
} };

bool Compiler::Compile( std::string_view sourceCode, Chunk* chunk )
{
  try
  {
    assert( chunk != nullptr );
    compilingChunk_ = chunk;
    lexer_.SetSource( sourceCode );
    lexer_.ExtractTokens(); // may throw; TODO early out for error
    currToken_ = std::begin( lexer_.GetTokens() );
    //Advance();
    Expression();
    Consume( TokenType::EndOfFile, "Expected end of expression" );
    EmitByte( OpCode::Return ); // endCompiler -> emitReturn -> emitByte TODO
  }
  catch( ... )
  {
#if defined(DEBUG_PRINT_CODE)
    GetCurrentChunk()->Disassemble( "code" );
#endif
    throw;
  }
  return true;
}

void Compiler::Grouping()
{
  Expression();
  Consume( TokenType::CloseParen, "Expected ')' after expression" );
}

void Compiler::Number()
{
  Value value{ *prevToken_ };
  EmitConstant( value );
}

#pragma warning(push)
#pragma warning(disable: 4061)
void Compiler::Unary()
{
  TokenType operatorType = prevToken_->GetType();
  ParsePrecedence( Precedence::Unary );
  switch( operatorType )
  {
  case TokenType::Minus: EmitByte( OpCode::Negate ); break;
  default: return;
  }
}
#pragma warning(pop)

void Compiler::Binary()
{
  TokenType operatorType = prevToken_->GetType();
  const ParseRule& rule = GetRule( operatorType );
  Precedence pr = rule.precedence_;
  ParsePrecedence( ++pr );

  switch( operatorType )
  {
  case TokenType::Plus:     EmitByte( OpCode::Add );      break;
  case TokenType::Minus:    EmitByte( OpCode::Subtract ); break;
  case TokenType::Multiply: EmitByte( OpCode::Multiply ); break;
  case TokenType::Divide:   EmitByte( OpCode::Divide );   break;
  }
}

void Compiler::Advance()
{
  prevToken_ = currToken_;
  ++currToken_;
}

void Compiler::Expression()
{
  ParsePrecedence( Precedence::Assignment );
}

void Compiler::Consume( TokenType tokenType, std::string_view errMsg )
{
  if( currToken_->GetType() == tokenType )
  {
    Advance();
    return;
  }
  throw CompilerError( errMsg, *currToken_ );
}

void Compiler::ParsePrecedence( Precedence precedence )
{
  // By definition the first token always belongs to a prefix expression
  Advance();
  ParseFn prefix = GetPrefixFn();
  if( prefix == nullptr )
    throw CompilerError{ "Expected an expression", *prevToken_ };

  ( this->*prefix )( );

  // Look for an infix parser for the next token. If the next token
  // is too low precendece or isn't an infix operator, we're done
  while( precedence <= GetRule( currToken_->GetType() ).precedence_ )
  {
    Advance();
    ParseFn infix = GetInfixFn();
    assert( infix != nullptr );
    ( this->*infix )( );
  }

  /*
  TokenType currTokenType = currToken_->GetType();
  rule = GetRule( currTokenType );
  
  for( Precedence currTokenPrecedence = rule.precedence_;
       precedence <= currTokenPrecedence;
       currTokenPrecedence = rule.precedence_ )
  {
    Advance();
    prevTokenType = prevToken_->GetType();
    rule = GetRule( prevTokenType );
    ParseFn infix = rule.infix_;
    ( this->*infix )( );
    rule = GetRule( currTokenType );
  }
  */
}

Compiler::ParseFn Compiler::GetPrefixFn() const
{
  TokenType prevTokenType = prevToken_->GetType();
  ParseRule rule = GetRule( prevTokenType );
  return rule.prefix_;
}

Compiler::ParseFn Compiler::GetInfixFn() const
{
  TokenType prevTokenType = prevToken_->GetType();
  ParseRule rule = GetRule( prevTokenType );
  return rule.infix_;
}

const Compiler::ParseRule& Compiler::GetRule( TokenType tokenType ) const
{
  // TODO use frozen
  assert( tokenType < TokenType::Last );
  size_t index = static_cast<size_t>( tokenType );
  return kParseRules[ index ];
}

void Compiler::EmitConstant( Value value )
{
  EmitBytes( OpCode::Constant, MakeConstant( value ) );
}

uint8_t Compiler::MakeConstant( Value value ) // TODO rename GetConstantOffset
{
  return GetCurrentChunk()->AddConstant( value.GetInt() );
}

void Compiler::EmitByte( OpCode opCode )
{
  EmitByte( static_cast<uint8_t>( opCode ) );
}

void Compiler::EmitByte( uint8_t byte )
{
  GetCurrentChunk()->Append(byte, 0 /* TODO prevToken_->GetLine() */);
}

void Compiler::EmitBytes( OpCode opCode, uint8_t byte )
{
  EmitByte( opCode );
  EmitByte( byte );
}


///////////////////////////////////////////////////////////////////////////////
