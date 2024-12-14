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
  {&Compiler::Variable, nullptr,            Precedence::None},        // Identifier
  {&Compiler::String,   nullptr,            Precedence::None},        // String
  {nullptr,             &Compiler::And,     Precedence::And},         // And
  {nullptr,             &Compiler::Or,      Precedence::Or},          // Or
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

void Compiler::Grouping(bool)
{
  Expression();
  Consume( TokenType::CloseParen, "Expected ')' after expression" );
}

void Compiler::Number(bool)
{
  Value value{ *prevToken_ };
  EmitConstant( value );
}

#pragma warning(push)
#pragma warning(disable: 4061)
void Compiler::Unary(bool)
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

void Compiler::Binary(bool)
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

void Compiler::Literal(bool)
{
  TokenType operatorType = prevToken_->GetType();
  switch( operatorType )
  {
  case TokenType::True:  EmitByte( OpCode::True );  break;
  case TokenType::False: EmitByte( OpCode::False ); break;
  }
}

void Compiler::String(bool)
{
  std::string_view lexeme = prevToken_->GetValue();
  Value str{ lexeme };
  EmitConstant( str );
}

void Compiler::Variable( bool canAssign )
{
  // TODO DefineVariable?
  std::string_view lexeme = prevToken_->GetValue();
  NamedVariable( lexeme, canAssign );
}

void Compiler::And( bool /*canAssign*/ )
{
  // At this point, LHS expression has already been compiled
  // If LHS is false, then skip over right operand
  uint32_t endJump = EmitJump( OpCode::JumpIfFalse );
  EmitByte( OpCode::Pop );
  ParsePrecedence( Precedence::And );
  PatchJump( endJump );
}

void Compiler::Or( bool /*canAssign*/ )
{
  // At this point, LHS expression has already been compiled
  // If LHS is true, then skip over right operand
  // TODO add JumpIfTrue and refactor this code https://craftinginterpreters.com/jumping-back-and-forth.html
  uint32_t elseJump = EmitJump( OpCode::JumpIfFalse );
  uint32_t endJump = EmitJump( OpCode::Jump );
  PatchJump( elseJump );
  EmitByte( OpCode::Pop );
  ParsePrecedence( Precedence::Or );
  PatchJump( endJump );
}

void Compiler::NamedVariable( std::string_view varName, bool canAssign )
{
  OpCode getOp, setOp;
  uint8_t index = 0;
  if( ResolveLocal( varName, index ) ) // sets index
  {
    getOp = OpCode::GetLocal;
    setOp = OpCode::SetLocal;
  }
  else
  {
    index = IdentifierConstant( varName );
    getOp = OpCode::GetGlobal;
    setOp = OpCode::SetGlobal;
  }

  if( canAssign && Match( TokenType::Assign ) )
  {
    Expression();
    EmitBytes( setOp, index );
  }
  else
  {
    EmitBytes( getOp, index );
  }
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

void Compiler::Block()
{
  while( !Check( TokenType::CloseBrace ) && !Check( TokenType::EndOfFile ) )
    Declaration();
  Consume( TokenType::CloseBrace, "Expected '}' after block" );
}

void Compiler::VarDeclaration()
{
  TokenType variableType = prevToken_->GetType();
  auto index = ParseVariable( "Expected variable name" );
  if( Match( TokenType::Assign ) ) // var x = expression
    Expression();
  else // var x; set to appropriate zero equivalent
    EmitConstant( GetEmptyValue( variableType ) );

  Consume( TokenType::EndStatement, "Expected ';' after variable declaration" );
  DefineVariable( index );
}

void Compiler::ExpressionStatement()
{
  Expression();
  Consume( TokenType::EndStatement, "Expected ';' after expression" );
  EmitByte( OpCode::Pop );
}

void Compiler::IfStatement()
{
  Consume( TokenType::OpenParen, "Expected '(' after if statement" );
  Expression();
  Consume( TokenType::CloseParen, "Expected ')' after condition" );
  uint32_t thenBytes = EmitJump( OpCode::JumpIfFalse );
  EmitByte( OpCode::Pop );
  Statement();
  uint32_t elseBytes = EmitJump( OpCode::Jump );
  PatchJump( thenBytes );
  EmitByte( OpCode::Pop );
  if( Match( TokenType::Else ) )
    Statement();
  PatchJump( elseBytes );
}

void Compiler::PrintStatement()
{
  Expression();
  Consume( TokenType::EndStatement, "Expected ';' after value" );
  EmitByte( OpCode::Print );
}

void Compiler::WhileStatement()
{
  uint32_t loopStart = GetCurrentChunk()->GetCodeByteCount();
  Consume( TokenType::OpenParen, "Expected '(' after 'while'" );
  Expression();
  Consume( TokenType::CloseParen, "Expected ')' after condition" );

  uint32_t exitJump = EmitJump( OpCode::JumpIfFalse );
  EmitByte( OpCode::Pop );
  Statement();
  EmitLoop( loopStart ); // jump back to condition

  PatchJump( exitJump );
  EmitByte( OpCode::Pop );
}

void Compiler::ForStatement()
{
  // Ensure that variables declared in initializer clause are locally scoped
  BeginScope();

  // Initializer clause
  Consume( TokenType::OpenParen, "Expected '(' after 'for'" );
  if( Match( TokenType::EndStatement ) )
    ; // empty
  else if( Match( TokenType::Int, TokenType::Char, TokenType::Str, TokenType::Bool ) )
    VarDeclaration(); // e.g. int x = 0;
  else
    ExpressionStatement(); // e.g. x = 0;

  // Condition clause
  uint32_t loopStart = GetCurrentChunk()->GetCodeByteCount();
  uint32_t exitJump = 0;
  bool hasConditionClause = !Match( TokenType::EndStatement );
  if( hasConditionClause )
  {
    Expression();
    Consume( TokenType::EndStatement, "Expected second ';' in 'for'" );

    // Exit loop if condition is false
    exitJump = EmitJump( OpCode::JumpIfFalse );
    EmitByte( OpCode::Pop ); // Condition
  }

  // Increment clause
  if( !Match( TokenType::CloseParen ) )
  {
    // Compile the increment, but don't execute it yet
    uint32_t bodyJump = EmitJump( OpCode::Jump );
    uint32_t incrementStart = GetCurrentChunk()->GetCodeByteCount();
    Expression();
    EmitByte( OpCode::Pop );
    Consume( TokenType::CloseParen, "Expected ')' after 'for' clause" );

    // Return to the condition expression
    EmitLoop( loopStart );

    // Enable the jump to the increment expression below
    loopStart = incrementStart;
    PatchJump( bodyJump );
  }

  // Loop body
  Statement();
  EmitLoop( loopStart );

  if( hasConditionClause )
  {
    PatchJump( exitJump );
    EmitByte( OpCode::Pop );
  }
  EndScope();
}

void Compiler::Declaration()
{
  if( Match( TokenType::Str, TokenType::Int, TokenType::Bool, TokenType::Char ) )
    VarDeclaration();
  else
    Statement();
}

void Compiler::Statement()
{
  if( Match( TokenType::Print ) )
    PrintStatement();
  else if( Match( TokenType::For ) )
    ForStatement();
  else if( Match( TokenType::If ) )
    IfStatement();
  else if( Match( TokenType::While ) )
    WhileStatement();
  else if( Match( TokenType::OpenBrace ) )
  {
    BeginScope();
    Block();
    EndScope();
  }
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

bool Compiler::Check( TokenType tokenType ) // TODO need this or fold into existing code? TODO IsTokenMatch
{
  return currToken_->GetType() == tokenType;
}

void Compiler::ParsePrecedence( Precedence precedence )
{
  // By definition the first token always belongs to a prefix expression
  Advance();
  ParseFn prefix = GetPrefixFn();
  if( prefix == nullptr )
    throw CompilerError{ "Expected an expression", *prevToken_ };

  bool canAssign = ( precedence <= Precedence::Assignment );
  ( this->*prefix )( canAssign );

  // Look for an infix parser for the next token. If the next token
  // is too low precendece or isn't an infix operator, we're done
  while( precedence <= GetRule( currToken_->GetType() ).precedence_ )
  {
    Advance();
    ParseFn infix = GetInfixFn();
    assert( infix != nullptr );
    ( this->*infix )( canAssign );
  }

  if( canAssign && Match( TokenType::Assign ) )
    throw CompilerError{ "Invalid assignment target", *prevToken_ };

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

uint8_t Compiler::IdentifierConstant( std::string_view identifierName )
{
  Value value{ identifierName };
  return MakeConstant( value );
}

bool Compiler::ResolveLocal( std::string_view identifierName, uint8_t& index ) // TODO FindLocal
{
  if( comp_.localCount == 0 )
    return false; // no locals to resolve

  // TODO replace with std::array, iterate in reverse order
  for( int i = comp_.localCount - 1; i >= 0; --i )
  {
    const Local* local = &comp_.locals[i];
    if( identifierName == local->token.GetValue() )
    {
      if( !local->isInitialized )
        throw CompilerError( "Can't read local variable in its own initializer" );
      index = static_cast<uint8_t>(i);
      return true;
    }
  }
  return false;
}

void Compiler::AddLocal( Token token )
{
  if( comp_.localCount >= 255 )
    throw CompilerError( "Too many local variables in function" );

  Local* local = &comp_.locals[comp_.localCount++];
  local->token = token;
  local->depth = comp_.scopeDepth;
}

void Compiler::DeclareVariable()
{
  if( comp_.scopeDepth == 0 ) // global scope
    return;

  // Local scope; check for duplicates
  Token token = *prevToken_;
  if( comp_.localCount >= 0 )
  {
    for( int i = comp_.localCount - 1; i >= 0; --i )
    {
      Local* local = &comp_.locals[i];
      if( local->depth != -1 && local->depth < comp_.scopeDepth )
        break;
      if( token.GetValue() == local->token.GetValue() )
        throw CompilerError( "Already a variable with this name in scope" );
    }
  }
  AddLocal( token );
}

uint8_t Compiler::ParseVariable( std::string_view errMsg )
{
  Consume( TokenType::Identifier, errMsg );
  DeclareVariable();

  // Exit if local scope, else define a global
  return ( comp_.scopeDepth > 0 ) ? uint8_t(0) : IdentifierConstant( prevToken_->GetValue() );
}

void Compiler::DefineVariable( uint8_t global )
{
  if( comp_.scopeDepth > 0 ) // local scope
  {
    comp_.MarkInitialized();
    return;
  }
  EmitBytes( OpCode::DefineGlobal, global );
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

void Compiler::EmitLoop( uint32_t loopStart )
{
  EmitByte( OpCode::Loop );

  uint32_t offset = GetCurrentChunk()->GetCodeByteCount();
  assert( loopStart <= offset );
  offset -= loopStart;
  offset += 2; // size of the OpCode::Loop operands
  if( offset > std::numeric_limits<uint16_t>::max() )
    throw CompilerError( "Loop body too large" );

  EmitByte( ( offset >> 8 ) & 0xFF ); // hi
  EmitByte( ( offset >> 0 ) & 0xFF ); // lo
}

uint32_t Compiler::EmitJump( OpCode opCode )
{
  EmitByte( opCode ); // TODO EmitBytes(opCode, 0xFFFF) or EmitBytes(opCode, 0xFF, 0xFF)
  EmitByte( 0xFF ); // 16-bit placeholder for backpatching
  EmitByte( 0xFF );
  return GetCurrentChunk()->GetCodeByteCount() - 2; // can we get rid of -2 here and patchjump? TODO
}

void Compiler::PatchJump( uint32_t offset )
{
  Chunk* chunk = GetCurrentChunk();
  uint32_t jumpBytes = chunk->GetCodeByteCount() - offset - 2;
  if( jumpBytes > std::numeric_limits<uint16_t>::max() )
    throw CompilerError( "Too much code to jump over" );

  uint8_t* code = chunk->GetCode();
  code[offset++] = static_cast<uint8_t>( ( jumpBytes >> 8 ) & 0xFF ); // hi
  code[offset++] = static_cast<uint8_t>( ( jumpBytes >> 0 ) & 0xFF ); // lo
}

void Compiler::BeginScope()
{
  ++comp_.scopeDepth; // TODO int32_t
}

void Compiler::EndScope()
{
  assert( comp_.scopeDepth > 0 );
  --comp_.scopeDepth;

  // Discard any variables in the scope we just ended
  while( comp_.localCount > 0 && 
         comp_.locals[comp_.localCount - 1].depth > comp_.scopeDepth )
  {
    EmitByte( OpCode::Pop );
    --comp_.localCount;
  }
}

Value Compiler::GetEmptyValue( TokenType tokenType ) // static
{
  switch( tokenType )
  {
  case TokenType::Str:  return Value{ std::string{} };
  case TokenType::Int:  return Value{ 0 };
  case TokenType::Bool: return Value{ false };
  case TokenType::Char: return Value{ '\0' };
  }
  return {};
}

///////////////////////////////////////////////////////////////////////////////
