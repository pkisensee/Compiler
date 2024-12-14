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

}; // enum class Precedence

inline Precedence& operator++( Precedence& precedence )
{
  return precedence = static_cast<Precedence>( static_cast<int>( precedence ) + 1 );
}

enum class FunctionType
{
  Function,
  Script,
  Max
}; // enum class FunctionType

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

  typedef void ( Compiler::*ParseFn )( bool canAssign );
  class ParseRule
  {
  public:
    ParseFn prefix_ = nullptr;
    ParseFn infix_ = nullptr;
    Precedence precedence_ = Precedence::None;
  };

  struct Local
  {
    Token token;
    bool isInitialized = false;
    uint8_t depth = 0;
  };

  struct Comp
  {
    Function* function;
    FunctionType functionType;
    Local locals[255]; // TODO
    uint8_t localCount = 0;
    uint8_t scopeDepth = 0; // zero is global scope

    void MarkInitialized()
    {
      assert( localCount > 0 );
      uint8_t localIndex = static_cast<uint8_t>( localCount - 1 );
      locals[localIndex].isInitialized = true;
      locals[localIndex].depth = scopeDepth;
    }
  };

public:
  void Grouping( bool );
  void Number( bool );
  void Unary( bool );
  void Binary( bool );
  void Literal( bool );
  void String( bool );
  void NamedVariable( std::string_view, bool canAssign );
  void Variable( bool canAssign );
  void And( bool canAssign );
  void Or( bool canAssign );

private:

  Chunk* GetCurrentChunk()
  {
    return compilingChunk_;
  }

  void Advance();
  void Expression();
  void Block();
  void VarDeclaration();
  void ExpressionStatement();
  void IfStatement();
  void PrintStatement();
  void WhileStatement();
  void ForStatement();
  void Declaration();
  void Statement();
  void Consume( TokenType, std::string_view );
  bool Check( TokenType );

  // Determine if the current token matches any of the input tokens and advance if so
  template<typename... TokenTypes>
  bool Match( TokenTypes... tokenTypes )
  {
    std::initializer_list<TokenType> tokenTypeList{ tokenTypes... };
    for( const auto& tokenType : tokenTypeList )
    {
      if( Check( tokenType ) )
      {
        Advance();
        return true;
      }
    }
    return false;
  }

  void ParsePrecedence( Precedence );
  uint8_t IdentifierConstant( std::string_view );
  bool ResolveLocal( std::string_view, uint8_t& );
  void AddLocal( Token );
  void DeclareVariable();
  uint8_t ParseVariable( std::string_view );
  void DefineVariable( uint8_t );

  ParseFn GetPrefixFn() const;
  ParseFn GetInfixFn() const;
  const ParseRule& GetRule( TokenType ) const;

  void EmitConstant( Value );
  uint8_t MakeConstant( Value );

  void EmitByte( OpCode );
  void EmitByte( uint8_t );
  void EmitBytes( OpCode, OpCode );
  void EmitBytes( OpCode, uint8_t );
  void EmitLoop( uint32_t );
  uint32_t EmitJump( OpCode );
  void PatchJump( uint32_t );

  void BeginScope();
  void EndScope();

  static Value GetEmptyValue( TokenType );

private:

  Lexer lexer_;
  TokenList::const_iterator prevToken_;
  TokenList::const_iterator currToken_;
  Chunk* compilingChunk_ = nullptr;
  Comp comp_;

}; // class Compiler

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
