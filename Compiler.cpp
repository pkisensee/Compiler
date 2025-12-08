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
#include <memory>
#include <utility>
#include <vector>

#include "ByteCodeBlock.h"
#include "Compiler.h"
#include "CompilerError.h"
#include "Function.h"
#include "Lexer.h"
#include "Token.h"
#include "UpvalueRef.h"
#include "Util.h"
#include "Value.h"

using namespace PKIsensee;

// TODO add frozen array with opcode names

// TODO improve so this isn't order dependent
std::array<Compiler::ParseRule, static_cast<size_t>(TokenType::Last)> kParseRules =
{ {
  {nullptr,             nullptr,            Precedence::None},        // OpenBracket
  {nullptr,             nullptr,            Precedence::None},        // CloseBracket
  {nullptr,             nullptr,            Precedence::None},        // OpenBrace
  {nullptr,             nullptr,            Precedence::None},        // CloseBrace
  {&Compiler::Grouping, &Compiler::Call,    Precedence::Call},        // OpenParen
  {nullptr,             nullptr,            Precedence::None},        // CloseParen
  {nullptr,             &Compiler::Binary,  Precedence::Comparison},  // LessThan
  {nullptr,             &Compiler::Binary,  Precedence::Comparison},  // GreaterThan
  {nullptr,             nullptr,            Precedence::None},        // EndStatement
  {nullptr,             nullptr,            Precedence::None},        // Assign
  {nullptr,             &Compiler::Binary,  Precedence::Add },        // Plus
  {&Compiler::Unary,    &Compiler::Binary,  Precedence::Add },        // Minus
  {nullptr,             &Compiler::Binary,  Precedence::Mult},        // Multiply
  {nullptr,             &Compiler::Binary,  Precedence::Mult},        // Divide
  {nullptr,             &Compiler::Binary,  Precedence::Mult},        // Modulus
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
  {&Compiler::Variable, nullptr,            Precedence::None},        // FuncPtr
  {nullptr,             nullptr,            Precedence::None},        // Invalid
  {nullptr,             nullptr,            Precedence::None},        // EndOfFile
} };

Compiler::Compiler()
{
  compStack_.push_back( &root_ );
}

Compiler::Compiler( FunctionType fnType, std::string_view fnName )
{
  compStack_.push_back( &root_ );

  GetC().SetFunctionType( fnType );
  if( fnType != FunctionType::Script )
    GetC().GetFunction().SetName(fnName);
}

Function Compiler::Compile( std::string_view sourceCode )
{
  try
  {
    assert( GetC().GetFunction().GetByteCodeBlock() != nullptr);
    lexer_.SetSource( sourceCode );
    lexer_.ExtractTokens(); // may throw; TODO early out for error
    currToken_ = std::begin( lexer_.GetTokens() ); // handle case with no tokens
    while( !Match( TokenType::EndOfFile ) ) // TODO better name
      Declaration();
    EmitReturn();
    GetCurrentByteCodeBlock()->Disassemble( GetC().GetFunction().GetName());
    return GetC().GetFunction();
  }
  catch ( CompilerError& compilerError )
  {
    std::cout << "  !! Exception thrown: " << compilerError.GetErrorMessage() << '\n';
    GetCurrentByteCodeBlock()->Disassemble( GetC().GetFunction().GetName() );
    return GetC().GetFunction();
  }
  catch ( ... )
  {
    std::cout << "  !! Unknown exception thrown\n";
    GetCurrentByteCodeBlock()->Disassemble( GetC().GetFunction().GetName() );
    throw;
  }
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
  case TokenType::Modulus:  EmitByte( OpCode::Modulus );  break;
  }
}

void Compiler::Call( bool )
{
  auto argCount = ArgumentList();
  //EmitDebug( "Call", ' ', std::format("args={}", argCount ) );
  EmitBytes( OpCode::Call, argCount );
}
  
void Compiler::Literal( bool )
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
  uint32_t index = 0;
  std::string_view getName;
  std::string_view setName;
  if( ResolveLocal( varName, index ) ) // sets index
  {
    getOp = OpCode::GetLocal;
    setOp = OpCode::SetLocal;
    getName = "GetLocal"; // TODO use frozen table
    setName = "SetLocal";
  }
  else if( ( ResolveUpvalue( varName, index ) ) ) // sets index
  {
    getOp = OpCode::GetUpvalue;
    setOp = OpCode::SetUpvalue;
    getName = "GetUpvalue"; // TODO use frozen table
    setName = "SetUpvalue";
  }
  else
  {
    index = IdentifierConstant( varName );
    getOp = OpCode::GetGlobal;
    setOp = OpCode::SetGlobal;
    getName = "GetGlobal";
    setName = "SetGlobal";
  }

  if ( index > std::numeric_limits<uint8_t>::max() )
    throw CompilerError( std::format( "Can't exceed {} variables", std::numeric_limits<uint8_t>::max() ) );
  uint8_t index8 = static_cast<uint8_t>( index );

  if( canAssign && Match( TokenType::Assign ) )
  {
    Expression();
    //EmitDebug( setName, ' ', varName);
    EmitBytes( setOp, index8 );
  }
  else
  {
    //EmitDebug( getName, ' ', varName);
    EmitBytes( getOp, index8 );
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

void Compiler::FunctionCall()
{
  // TODO if we throw from this function (which can definitely happen), then the value on
  // the top of the compStack refers to stack-based memory that has gone out of scope, 
  // which is evil! Need to revist this pattern.
  FunctionInfo comp; // TODO name FunctionInfo
  compStack_.push_back( &comp );

  // TODO should the fn name param (GetValue()) be a std::string? Does the original source persist?
  // See https://craftinginterpreters.com/calls-and-functions.html#function-parameters
  GetC().SetFunctionType( FunctionType::Function );
  GetC().GetFunction().SetName(prevToken_->GetValue());
  BeginScope();
  Consume( TokenType::OpenParen, "Expected '(' after function name" );
  if( !Check( TokenType::CloseParen ) )
  {
    do {
      GetC().GetFunction().IncrementParamCount();

      if( !Match( TokenType::Str, TokenType::Int, TokenType::Bool, TokenType::Char ) )
        throw CompilerError( "Expected parameter type" );

      // Semantically a parameter is a local variable declared in the function
      TokenType variableType = prevToken_->GetType();
      (void)variableType; // TODO pass this to the appropriate place
      std::string_view paramName;
      auto index = ParseVariable( "Expected parameter name", paramName );
      DefineVariable( index, paramName );
    } while( Match( TokenType::Comma ) );
  }
  Consume( TokenType::CloseParen, "Expected ')' after parameters" );
  Consume( TokenType::OpenBrace, "Expected '{' before function body" );
  Block();

  // ObjFunction* function = endCompiler(); // current->function LOX
  Function function = GetC().GetFunction();
  Value closure( Closure( GetC().GetFunction() ) );
  EmitReturn();
  GetCurrentByteCodeBlock()->Disassemble( function.GetName() );
  compStack_.pop_back();

  // Store reference to this closure in the caller's constant table
  EmitBytes( OpCode::Closure, MakeConstant( closure ) );

  // Store any upvalues we captured from this function
  for( uint32_t i = 0u; i < function.GetUpvalueCount(); ++i )
  {
    UpvalueRef upValue = comp.GetUpvalue( i );
    EmitByte( upValue.IsLocal() );
    EmitByte( upValue.GetIndexAsByte() );
  }
}

void Compiler::FunctionDeclaration()
{
  std::string_view funcName;
  auto global = ParseVariable( "Expected function name", funcName );
  GetC().MarkInitialized();
  FunctionCall();
  DefineVariable( global, funcName );
}

void Compiler::VarDeclaration()
{
  TokenType variableType = prevToken_->GetType();
  std::string_view varName;
  auto index = ParseVariable( "Expected variable name", varName );
  if( Match( TokenType::Assign ) ) // varType varName = expression
    Expression();
  else // varType varName; set to appropriate zero equivalent
    EmitConstant( GetEmptyValue( variableType ) );

  Consume( TokenType::EndStatement, "Expected ';' after variable declaration" );
  DefineVariable( index, varName );
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

void Compiler::ReturnStatement()
{
  if( GetC().GetFunctionType() == FunctionType::Script)
    throw CompilerError( "Top level code may not return" );

  if( Match( TokenType::EndStatement ) )
    EmitByte( OpCode::Empty ); // placeholder for no return value
  else
  {
    Expression(); // return value
    Consume( TokenType::EndStatement, "Expected ';' after return value" );
  }
  EmitByte( OpCode::Return );
}
  
void Compiler::PrintStatement()
{
  Expression();
  Consume( TokenType::EndStatement, "Expected ';' after value" );
  EmitByte( OpCode::Print );
}

void Compiler::WhileStatement()
{
  uint32_t loopStart = GetCurrentByteCodeBlock()->GetSize();
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
  uint32_t loopStart = GetCurrentByteCodeBlock()->GetSize();
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
    uint32_t incrementStart = GetCurrentByteCodeBlock()->GetSize();
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
  if( Match( TokenType::Function ) )
    FunctionDeclaration();
  else if( Match( TokenType::Str, TokenType::Int, TokenType::Bool, TokenType::Char, TokenType::FunRef ) )
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
  else if( Match( TokenType::Return ) )
    ReturnStatement();
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

bool Compiler::ResolveLocal( std::string_view identifierName, uint32_t& index ) // TODO FindLocal
{
  return GetC().ResolveLocal( identifierName, index );
}

bool Compiler::ResolveUpvalue( std::string_view identifierName, uint32_t& index )
{
  // No upvalues at global scope
  if( GetC().GetFunctionType() == FunctionType::Script)
    return false;

  // Capture the variable if it's in the enclosing scope
  if( GetC(0).ResolveLocal( identifierName, index ) )
  {
    GetC(0).AddUpvalue( index, true );
    return true;
  }

  // Recursively resolve locals and upvalues in all enclosing scopes inner to outer
  return RecursiveResolveUpvalue( identifierName, index, 0 );
}

bool Compiler::RecursiveResolveUpvalue( std::string_view identifierName, uint32_t& index, uint8_t scope )
{
  if( scope+1u >= GetScopeCount() )
    return false;

  // If local exists in this scope, add upvalue to previous inner scope
  if( GetC( scope+1u ).ResolveLocal( identifierName, index ) )
  {
    GetC( scope ).AddUpvalue( index, true );
    return true;
  }

  // Recurse to next level scope
  if( RecursiveResolveUpvalue( identifierName, index, scope+1u ) )
  {
    GetC( scope ).AddUpvalue( index, false );
    return true;
  }

  return false;
}

void Compiler::AddLocal( Token token )
{
  GetC().AddLocal( token ); // TODO GetC -> something else
}

void Compiler::DeclareVariable()
{
  if( GetC().GetScopeDepth() == 0 ) // global scope
    return;

  // Local scope
  Token token = *prevToken_;
  AddLocal( token );
}

uint8_t Compiler::ParseVariable( std::string_view errMsg, std::string_view& varName )
{
  Consume( TokenType::Identifier, errMsg );
  varName = prevToken_->GetValue();
  DeclareVariable();

  // Exit if local scope
  if( GetC().GetScopeDepth() > 0 )
    return 0;

  // Define a global
  return IdentifierConstant( varName );
}

void Compiler::DefineVariable( uint8_t global, std::string_view /*name*/ )
{
  if( GetC().GetScopeDepth() > 0 ) // local scope
  {
    GetC().MarkInitialized();
    return;
  }
  //EmitDebug( "DefineGlobal", ' ', name );
  // TODO store name somewhere so we can reference it later for debugging
  EmitBytes( OpCode::DefineGlobal, global );
}

uint8_t Compiler::ArgumentList()
{
  uint8_t argCount = 0;
  if( !Check( TokenType::CloseParen ) )
  {
    do {
      Expression();
      ++argCount;
      if( argCount == 0 ) // uint8_t limitation TODO constant
        throw CompilerError( "Can't have more than 255 arguments" );
    } while( Match( TokenType::Comma ) );
  }
  Consume( TokenType::CloseParen, "Expected ')' after arguments" );
  return argCount;
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
  //EmitDebug( "Constant: ", value );
  EmitBytes( OpCode::Constant, MakeConstant( value ) );
}

uint8_t Compiler::MakeConstant( Value value ) // TODO rename GetConstantIndex
{
  return GetCurrentByteCodeBlock()->AddConstant( value );
}

void Compiler::EmitByte( OpCode opCode )
{
  std::string_view name;
  switch( opCode )
  {
  //case OpCode::Constant: name = "Constant"; break;
  //case OpCode::GetLocal: name = "GetLocal"; break;
  //case OpCode::GetGlobal: name = "GetGlobal"; break;
  //case OpCode::SetLocal: name = "SetLocal"; break;
  //case OpCode::DefineGlobal: name = "DefineGlobal"; break;
  //case OpCode::SetGlobal: name = "SetGlobal"; break;
  //case OpCode::Loop: name = "Loop"; break;
  //case OpCode::Call: name = "Call"; break;
  case OpCode::True: name = "True"; break;
  case OpCode::False: name = "False"; break;
  case OpCode::Empty: name = "Empty"; break;
  case OpCode::Pop: name = "Pop"; break;
  case OpCode::IsEqual: name = "IsEqual"; break;
  case OpCode::Greater: name = "Greater"; break;
  case OpCode::Less: name = "Less"; break;
  case OpCode::Add: name = "Add"; break;
  case OpCode::Subtract: name = "Subtract"; break;
  case OpCode::Multiply: name = "Multiply"; break;
  case OpCode::Divide: name = "Divide"; break;
  case OpCode::Negate: name = "Negate"; break;
  case OpCode::Not: name = "Not"; break;
  case OpCode::Print: name = "Print"; break;
  case OpCode::Jump: name = "Jump"; break;
  case OpCode::JumpIfFalse: name = "JumpIfFalse"; break;
  case OpCode::Return: name = "Return"; break;
  }
  //if( name.size() )
  //  EmitDebug( name );
  EmitByte( static_cast<uint8_t>( opCode ) );
}

void Compiler::EmitByte( uint8_t byte )
{
  GetCurrentByteCodeBlock()->Append( byte, 0 /* TODO prevToken_->GetLine() */ );
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
  uint32_t offset = GetCurrentByteCodeBlock()->GetSize();
  assert( loopStart <= offset );
  offset -= loopStart;
  offset += 2; // size of the OpCode::Loop operands
  if( offset > std::numeric_limits<uint16_t>::max() )
    throw CompilerError( "Loop body too large" );

  //EmitDebug( "Loop offset: ", offset );
  EmitByte( ( offset >> 8 ) & 0xFF ); // hi TODO EmitWord
  EmitByte( ( offset >> 0 ) & 0xFF ); // lo
}

uint32_t Compiler::EmitJump( OpCode opCode )
{
  EmitByte( opCode ); // TODO EmitBytes(opCode, 0xFFFF) or EmitBytes(opCode, 0xFF, 0xFF)
  EmitByte( 0xFF ); // 16-bit placeholder for backpatching
  EmitByte( 0xFF );
  return GetCurrentByteCodeBlock()->GetSize() - 2; // can we get rid of -2 here and patchjump? TODO
}

void Compiler::PatchJump( uint32_t offset )
{
  ByteCodeBlock* byteCodeBlock = GetCurrentByteCodeBlock();
  uint32_t jumpBytes = byteCodeBlock->GetSize() - offset - 2;
  if( jumpBytes > std::numeric_limits<uint16_t>::max() )
    throw CompilerError( "Too much code to jump over" );

  auto code = byteCodeBlock->GetEntryPoint();
  code[offset++] = static_cast<uint8_t>( ( jumpBytes >> 8 ) & 0xFF ); // hi
  code[offset++] = static_cast<uint8_t>( ( jumpBytes >> 0 ) & 0xFF ); // lo
}

void Compiler::EmitReturn()
{
  EmitByte( OpCode::Empty );
  EmitByte( OpCode::Return );
}

void Compiler::BeginScope()
{
  GetC().IncrementScopeDepth();
}

void Compiler::EndScope()
{
  FunctionInfo& fnInfo = GetC();
  fnInfo.DecrementScopeDepth();

  uint32_t localCount = fnInfo.GetLocalCount();
  if ( localCount == 0 )
    return;

  // Discard any variables in the scope just ended
  uint8_t discardCount = 0;
  for ( uint32_t i = uint32_t(localCount-1); i > 0; --i ) // TODO range-based for
  {
    const Local& local = fnInfo.GetLocal( i );
    if ( local.GetDepth() > fnInfo.GetScopeDepth() )
    {
      EmitByte( OpCode::Pop );
      ++discardCount;
    }
  }

  fnInfo.SetLocalCount( localCount - discardCount );
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
