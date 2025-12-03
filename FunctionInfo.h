///////////////////////////////////////////////////////////////////////////////
//
//  FunctionInfo.h
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
#include "Function.h"
#include "Local.h"
#include "Token.h"

namespace PKIsensee
{

enum class FunctionType
{
  Function,
  Script,
  Max
}; // enum class FunctionType

struct UpvalueRef // TODO compress
{
  uint8_t index;
  bool isLocal;
};

class FunctionInfo
{
  static constexpr size_t kMaxLocals = 16;

public:
  UpvalueRef upValues_[ 255 ]; // TODO constant, std::array, minimize size; 16?
  uint8_t localCount_ = 1; // compiler claims slot zero for the VM's internal use
  uint8_t scopeDepth_ = 0; // zero is global scope

  FunctionInfo() = default;

  const Function& GetFunction() const
  {
    return function_;
  }

  Function& GetFunction()
  {
    return function_;
  }

  FunctionType GetFunctionType() const
  {
    return functionType_;
  }

  void SetFunctionType( FunctionType functionType )
  {
    functionType_ = functionType;
  }

  const Local& GetLocal( uint32_t i ) const
  {
    assert( i < kMaxLocals );
    assert( i < localCount_ );
    return locals_[ i ];
  }

  void AddLocal( Token token )
  {
    assert( localCount_ > 0 ); // compiler claims slot zero for the VM's internal use

    if (localCount_ >= kMaxLocals)
      throw CompilerError( "Too many local variables in function" );

    // Check for duplicates
    for (int i = localCount_ - 1; i >= 0; --i)
    {
      const Local& local = locals_[ i ];
      if ( local.GetDepth() != -1 && local.GetDepth() < scopeDepth_ )
        break;
      if ( token.GetValue() == local.GetToken().GetValue() )
        throw CompilerError( "Already a variable with this name in scope" );
    }

#pragma warning(push)
#pragma warning(disable : 6385)
    Local& local = locals_[ localCount_ ];
    local.SetLocal( token, scopeDepth_ );
    ++localCount_;
#pragma warning(pop)
  }

  void MarkInitialized()
  {
    if (scopeDepth_ == 0)
      return;
    assert( localCount_ > 0 );
    uint8_t localIndex = static_cast<uint8_t>( localCount_ - 1 );
    locals_[ localIndex ].SetInitialized( true );
    locals_[ localIndex ].SetDepth( scopeDepth_ );
  }

  bool ResolveLocal( std::string_view identifierName, uint32_t& index ) const // TODO FindLocal
  {
    if (localCount_ == 0)
      return false; // no locals to resolve

    // TODO replace with std::array, iterate in reverse order
    for (int i = localCount_ - 1; i >= 0; --i)
    {
      const Local* local = &locals_[ i ];
      if (identifierName == local->GetToken().GetValue())
      {
        if ( !local->IsInitialized() )
          throw CompilerError( "Can't read local variable in its own initializer" );
        index = static_cast<uint32_t>( i );
        return true;
      }
    }
    return false;
  }

  void AddUpvalue( uint32_t& index, bool isLocal )
  {
    auto upvalueCount = function_.GetUpvalueCount();

    // If function already has this upvalue, grab it
    // TODO std::find_if
    for (uint32_t i = 0u; i < upvalueCount; ++i)
    {
      if (upValues_[ i ].index == index && upValues_[ i ].isLocal == isLocal)
      {
        index = i;
        return;
      }
    }

    // New upvalue
    // TODO make a function for this and check bounds of incoming index
    upValues_[ upvalueCount ].isLocal = isLocal;
    upValues_[ upvalueCount ].index = static_cast<uint8_t>(index);
    index = function_.GetUpvalueCount();
    function_.IncrementUpvalueCount();
  }

  Local locals_[ FunctionInfo::kMaxLocals ]; // TODO private, constant, std::array; minisze size; 32?
private:
  Function function_; // TODO unique_ptr? TODO Closure
  FunctionType functionType_ = FunctionType::Script; // TODO FunctionType::GlobalScope?

}; // class FunctionInfo

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
