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

std::array<Compiler::ParseRule,static_cast<size_t>(TokenType::Last)> kParseRules =
{ {
  {&Compiler::Grouping, nullptr, Precedence::None}, // OpenBracket
  {nullptr, nullptr, Precedence::None}, // CloseBracket
  {nullptr, nullptr, Precedence::None}, // OpenBrace
  {nullptr, nullptr, Precedence::None}, // CloseBrace
  {nullptr, nullptr, Precedence::None}, // OpenParen
  {nullptr, nullptr, Precedence::None}, // CloseParen
  {nullptr, nullptr, Precedence::None}, // LessThan
  {nullptr, nullptr, Precedence::None}, // GreaterThan
  {nullptr, nullptr, Precedence::None}, // EndStatement
  {nullptr, nullptr, Precedence::None}, // Assign
  {nullptr, nullptr, Precedence::None}, // Plus
  {nullptr, nullptr, Precedence::None}, // Minus
  {nullptr, nullptr, Precedence::None}, // Multiply
  {nullptr, nullptr, Precedence::None}, // Divide
  {nullptr, nullptr, Precedence::None}, // Comma
  {nullptr, nullptr, Precedence::None}, // Dot
  {nullptr, nullptr, Precedence::None}, // IsEqual
  {nullptr, nullptr, Precedence::None}, // NotEqual
  {nullptr, nullptr, Precedence::None}, // LessThanEqual
  {nullptr, nullptr, Precedence::None}, // GreaterThanEqual
  {nullptr, nullptr, Precedence::None}, // Number
  {nullptr, nullptr, Precedence::None}, // Identifier
  {nullptr, nullptr, Precedence::None}, // String
  {nullptr, nullptr, Precedence::None}, // And
  {nullptr, nullptr, Precedence::None}, // Or
  {nullptr, nullptr, Precedence::None}, // Not
  {nullptr, nullptr, Precedence::None}, // If
  {nullptr, nullptr, Precedence::None}, // Else
  {nullptr, nullptr, Precedence::None}, // For
  {nullptr, nullptr, Precedence::None}, // While
  {nullptr, nullptr, Precedence::None}, // Return
  {nullptr, nullptr, Precedence::None}, // True
  {nullptr, nullptr, Precedence::None}, // False
  {nullptr, nullptr, Precedence::None}, // Print
  {nullptr, nullptr, Precedence::None}, // Str
  {nullptr, nullptr, Precedence::None}, // Int
  {nullptr, nullptr, Precedence::None}, // Char
  {nullptr, nullptr, Precedence::None}, // Bool
  {nullptr, nullptr, Precedence::None}, // Function
  {nullptr, nullptr, Precedence::None}, // Invalid
  {nullptr, nullptr, Precedence::None}, // EndOfFile
} };

bool Compiler::Compile( std::string_view sourceCode, Chunk* chunk )
{
  assert( chunk != nullptr );
  compilingChunk_ = chunk;
  lexer_.SetSource( sourceCode );
  lexer_.ExtractTokens(); // may throw; TODO early out for error
  currToken_ = std::begin( lexer_.GetTokens() );
  Advance();
  Expression();
  Consume( TokenType::EndOfFile, "Expected end of expression" );
  EmitByte( OpCode::Return ); // endCompiler -> emitReturn -> emitByte TODO
  return true;
}

void Compiler::Grouping()
{
  Expression();
  Consume( TokenType::CloseParen, "Expected '}' after expression" );
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

void Compiler::ParsePrecedence( Precedence )
{
  (void)0;
}

const Compiler::ParseRule& Compiler::GetRule( TokenType tokenType ) const
{
  // TODO use frozen
  return kParseRules[ size_t(tokenType) ];
}

void Compiler::EmitConstant( Value value )
{
  EmitBytes( OpCode::Constant, MakeConstant( value ) );
}

uint8_t Compiler::MakeConstant( Value value ) // TODO GetConstantOffset
{
  return currChunk_->AddConstant( value.GetInt() );
}

void Compiler::EmitByte( OpCode opCode )
{
  EmitByte( static_cast<uint8_t>( opCode ) );
}

void Compiler::EmitByte( uint8_t byte )
{
  currChunk_->Append( byte, 0 /* TODO prevToken_->GetLine() */);
}

void Compiler::EmitBytes( OpCode opCode, uint8_t byte )
{
  EmitByte( opCode );
  EmitByte( byte );
}


///////////////////////////////////////////////////////////////////////////////
