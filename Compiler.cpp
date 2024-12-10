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
#include "Util.h"
#include "Value.h"

using namespace PKIsensee;

// TODO improve so this isn't order dependent
std::array<Compiler::ParseRule, static_cast<size_t>(TokenType::Last)> kParseRules =
{ {
  {nullptr,             nullptr,            Precedence::None},        // OpenBracket
  {nullptr,             nullptr,            Precedence::None},        // CloseBracket
  {nullptr,             nullptr,            Precedence::None},        // OpenBrace
  {nullptr,             nullptr,            Precedence::None},        // CloseBrace
  {&Compiler::Grouping, nullptr,            Precedence::None},        // OpenParen
  {nullptr,             nullptr,            Precedence::None},        // CloseParen
  {nullptr,             &Compiler::Binary,  Precedence::Comparison},  // LessThan
  {nullptr,             &Compiler::Binary,  Precedence::Comparison},  // GreaterThan
  {nullptr,             nullptr,            Precedence::None},        // EndStatement
  {nullptr,             nullptr,            Precedence::None},        // Assign
  {nullptr,             &Compiler::Binary,  Precedence::Add },        // Plus
  {&Compiler::Unary,    &Compiler::Binary,  Precedence::Add },        // Minus
  {nullptr,             &Compiler::Binary,  Precedence::Mult},        // Multiply
  {nullptr,             &Compiler::Binary,  Precedence::Mult},        // Divide
  {nullptr,             nullptr,            Precedence::None},        // Comma
  {nullptr,             nullptr,            Precedence::None},        // Dot
  {nullptr,             &Compiler::Binary,  Precedence::Equality},    // IsEqual
  {nullptr,             &Compiler::Binary,  Precedence::Equality},    // NotEqual
  {nullptr,             &Compiler::Binary,  Precedence::Comparison},  // LessThanEqual
  {nullptr,             &Compiler::Binary,  Precedence::Comparison},  // GreaterThanEqual
  {&Compiler::Number,   nullptr,            Precedence::None},        // Number
  {nullptr,             nullptr,            Precedence::None},        // Identifier
  {&Compiler::String,   nullptr,            Precedence::None},        // String
  {nullptr,             nullptr,            Precedence::None},        // And
  {nullptr,             nullptr,            Precedence::None},        // Or
  {&Compiler::Unary,    nullptr,            Precedence::None},        // Not
  {nullptr,             nullptr,            Precedence::None},        // If
  {nullptr,             nullptr,            Precedence::None},        // Else
  {nullptr,             nullptr,            Precedence::None},        // For
  {nullptr,             nullptr,            Precedence::None},        // While
  {nullptr,             nullptr,            Precedence::None},        // Return
  {&Compiler::Literal,  nullptr,            Precedence::None},        // True
  {&Compiler::Literal,  nullptr,            Precedence::None},        // False
  {nullptr,             nullptr,            Precedence::None},        // Print
  {nullptr,             nullptr,            Precedence::None},        // Str
  {nullptr,             nullptr,            Precedence::None},        // Int
  {nullptr,             nullptr,            Precedence::None},        // Char
  {nullptr,             nullptr,            Precedence::None},        // Bool
  {nullptr,             nullptr,            Precedence::None},        // Function
  {nullptr,             nullptr,            Precedence::None},        // Invalid
  {nullptr,             nullptr,            Precedence::None},        // EndOfFile
} };

bool Compiler::Compile( std::string_view sourceCode, Chunk* chunk )
{
  try
  {
    assert( chunk != nullptr );
    compilingChunk_ = chunk;
    lexer_.SetSource( sourceCode );
    lexer_.ExtractTokens(); // may throw; TODO early out for error
    currToken_ = std::begin( lexer_.GetTokens() ); // handle case with no tokens
    while( !Match( TokenType::EndOfFile ) ) // TODO better name
      Declaration();
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
  case TokenType::Not:   EmitByte( OpCode::Not );    break;
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
  case TokenType::LessThan:         EmitByte( OpCode::Less ); break;
  case TokenType::GreaterThan:      EmitByte( OpCode::Greater ); break;
  case TokenType::IsEqual:          EmitByte( OpCode::IsEqual ); break;
  case TokenType::NotEqual:         EmitBytes( OpCode::IsEqual, OpCode::Not ); break;
  case TokenType::LessThanEqual:    EmitBytes( OpCode::Greater, OpCode::Not ); break;
  case TokenType::GreaterThanEqual: EmitBytes( OpCode::Less, OpCode::Not ); break;

  case TokenType::Plus:     EmitByte( OpCode::Add );      break;
  case TokenType::Minus:    EmitByte( OpCode::Subtract ); break;
  case TokenType::Multiply: EmitByte( OpCode::Multiply ); break;
  case TokenType::Divide:   EmitByte( OpCode::Divide );   break;
  }
}

void Compiler::Literal()
{
  TokenType operatorType = prevToken_->GetType();
  switch( operatorType )
  {
  case TokenType::True:  EmitByte( OpCode::True );  break;
  case TokenType::False: EmitByte( OpCode::False ); break;
  }
}

void Compiler::String()
{
  std::string_view lexeme = prevToken_->GetValue();
  Value str{ lexeme };
  EmitConstant( str );
}

void Compiler::Advance()
{
  // TODO this is wonky; ideally change to a for(i=beg;i!=end;++i)
  prevToken_ = currToken_;
  if( currToken_ != std::end( lexer_.GetTokens() ) )
    ++currToken_;
}

void Compiler::Expression()
{
  ParsePrecedence( Precedence::Assignment );
}

void Compiler::ExpressionStatement()
{
  Expression();
  Consume( TokenType::EndStatement, "Expected ';' after expression" );
  EmitByte( OpCode::Pop );
}

void Compiler::PrintStatement()
{
  Expression();
  Consume( TokenType::EndStatement, "Expected ';' after value" );
  EmitByte( OpCode::Print );
}

void Compiler::Declaration()
{
  Statement();
}

void Compiler::Statement()
{
  if( Match( TokenType::Print ) )
    PrintStatement();
  else
    ExpressionStatement();
}

void Compiler::Consume( TokenType tokenType, std::string_view errMsg )
{
  assert( currToken_ != std::end( lexer_.GetTokens() ) );
  if( currToken_->GetType() == tokenType )
  {
    Advance();
    return;
  }
  throw CompilerError( errMsg, *currToken_ );
}

bool Compiler::Check( TokenType tokenType ) // TODO need this or fold into existing code?
{
  return currToken_->GetType() == tokenType;
}

bool Compiler::Match( TokenType tokenType )
{
  if( !Check( tokenType ) )
    return false;
  Advance();
  return true;
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
  return GetCurrentChunk()->AddConstant( value );
}

void Compiler::EmitByte( OpCode opCode )
{
  EmitByte( static_cast<uint8_t>( opCode ) );
}

void Compiler::EmitByte( uint8_t byte )
{
  GetCurrentChunk()->Append(byte, 0 /* TODO prevToken_->GetLine() */);
}

void Compiler::EmitBytes( OpCode first, OpCode second ) // TODO varargs function to eliminate copy pasta
{
  EmitByte( first );
  EmitByte( second );
}

void Compiler::EmitBytes( OpCode opCode, uint8_t byte )
{
  EmitByte( opCode );
  EmitByte( byte );
}

///////////////////////////////////////////////////////////////////////////////
