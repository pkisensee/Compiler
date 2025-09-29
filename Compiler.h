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
#include <initializer_list>
#include <inplace_vector.h>
#include <string_view>

#include "CompilerError.h"
#include "FunctionInfo.h"
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

class ByteCodeBlock;

class Compiler
{
public:
  Compiler();
  Compiler( FunctionType, std::string_view fnName );

  // Disable copy/move
  Compiler( const Compiler& ) = delete;
  Compiler& operator=( const Compiler& ) = delete;
  Compiler( Compiler&& ) = delete;
  Compiler& operator=( Compiler&& ) = delete;

  Function Compile( std::string_view );
  void SetFunctionType( FunctionType funType )
  {
    GetC().SetFunctionType( funType );
  }

  typedef void ( Compiler::*ParseFn )( bool canAssign );
  class ParseRule
  {
  public:
    ParseFn prefix_ = nullptr;
    ParseFn infix_ = nullptr;
    Precedence precedence_ = Precedence::None;
  };

public:

  void Grouping( bool );
  void Number( bool );
  void Unary( bool );
  void Binary( bool );
  void Call( bool );
  void Literal( bool );
  void String( bool );
  void NamedVariable( std::string_view, bool canAssign );
  void Variable( bool canAssign );
  void And( bool canAssign );
  void Or( bool canAssign );

private:

  FunctionInfo& GetC()
  {
    return *compStack_.back();
  }

  const FunctionInfo& GetC() const
  {
    return *compStack_.back();
  }

  size_t GetScopeCount() const // includes current scope
  {
    return compStack_.size();
  }

  FunctionInfo& GetC( size_t i )
  {
    // i == 0 maps to current function (compStack_.top())
    // i == 1 maps to enclosing function
    // i == 2 maps to next level enclosing function
    // etc.
    assert( i < compStack_.size() );
    auto index = compStack_.size() - ( i+1 );
    return *compStack_[index];
  }

  ByteCodeBlock* GetCurrentByteCodeBlock()
  {
    return GetC().GetFunction().GetByteCodeBlock();
  }

  const ByteCodeBlock* GetCurrentByteCodeBlock() const
  {
    return GetC().GetFunction().GetByteCodeBlock();
  }

  void Advance();
  void Expression();
  void Block();
  void FunctionCall();
  void FunctionDeclaration();
  void VarDeclaration();
  void ExpressionStatement();
  void IfStatement();
  void ReturnStatement();
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
  bool ResolveUpvalue( std::string_view, uint8_t& );
  bool RecursiveResolveUpvalue( std::string_view, uint8_t&, uint8_t scope );
  void AddLocal( Token );
  void DeclareVariable();
  uint8_t ParseVariable( std::string_view, std::string_view& name );
  void DefineVariable( uint8_t, std::string_view name );
  uint8_t ArgumentList();

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
  void EmitReturn();

  void BeginScope();
  void EndScope();

  static Value GetEmptyValue( TokenType );
  std::string GetCurrOffset() const;

  template <typename Arg, typename... Args>
  void EmitDebug(Arg&& arg, Args&&... args)
  {
#if defined(DEBUG_PRINT_CODE)
    auto offset = GetCurrentByteCodeBlock()->GetCurrOffset();
    std::cout << std::format( "{:04d} ", offset );
    std::cout << std::forward<Arg>( arg );
    ( ( std::cout << std::forward<Args>( args ) ), ... );
    std::cout << '\n';
#endif
  }

private:

  Lexer lexer_;
  TokenList::const_iterator prevToken_;
  TokenList::const_iterator currToken_;
  inplace_vector<FunctionInfo*, 32> compStack_; // TODO define maximum function stack depth
  FunctionInfo root_;

}; // class Compiler

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
