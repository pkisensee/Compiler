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
#include "Token.h"

namespace PKIsensee
{

enum class FunctionType
{
  Function,
  Script,
  Max
}; // enum class FunctionType

struct Local // TODO compress
{
  Token token;
  uint8_t depth = 0;
  bool isInitialized = false;
};

struct UpvalueRef // TODO compress
{
  uint8_t index;
  bool isLocal;
};

class FunctionInfo
{
public:
  Local locals_[ 255 ]; // TODO constant, std::array; minisze size; 32?
  UpvalueRef upValues_[ 255 ]; // TODO constant, std::array, minimize size; 16?
  uint8_t localCount_ = 1; // the compiler claims slot zero for the VM's internal use
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

  void MarkInitialized()
  {
    if (scopeDepth_ == 0)
      return;
    assert( localCount_ > 0 );
    uint8_t localIndex = static_cast<uint8_t>( localCount_ - 1 );
    locals_[ localIndex ].isInitialized = true;
    locals_[ localIndex ].depth = scopeDepth_;
  }

  bool ResolveLocal( std::string_view identifierName, uint8_t& index ) const // TODO FindLocal
  {
    if (localCount_ == 0)
      return false; // no locals to resolve

    // TODO replace with std::array, iterate in reverse order
    for (int i = localCount_ - 1; i >= 0; --i)
    {
      const Local* local = &locals_[ i ];
      if (identifierName == local->token.GetValue())
      {
        if (!local->isInitialized)
          throw CompilerError( "Can't read local variable in its own initializer" );
        index = static_cast<uint8_t>( i );
        return true;
      }
    }
    return false;
  }

  void AddUpvalue( uint8_t& index, bool isLocal )
  {
    auto upvalueCount = function_.GetUpvalueCount();

    // If function already has this upvalue, grab it
    // TODO std::find_if
    for (uint32_t i = 0u; i < upvalueCount; ++i)
    {
      if (upValues_[ i ].index == index && upValues_[ i ].isLocal == isLocal)
      {
        index = static_cast<uint8_t>( i );
        return;
      }
    }

    // New upvalue
    upValues_[ upvalueCount ].isLocal = isLocal;
    upValues_[ upvalueCount ].index = index;
    index = function_.GetUpvalueCount();
    function_.IncrementUpvalueCount();
  }

private:
  Function function_; // TODO unique_ptr? TODO Closure
  FunctionType functionType_ = FunctionType::Script; // TODO FunctionType::GlobalScope?

}; // class FunctionInfo

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
