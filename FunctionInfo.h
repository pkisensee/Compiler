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
#include <array>
#include <inplace_vector.h>

#include "Function.h"
#include "Local.h"
#include "Token.h"
#include "Upvalue.h"

namespace PKIsensee
{

enum class FunctionType
{
  Function,
  Script,
  Max
}; // enum class FunctionType

class FunctionInfo
{
  static constexpr size_t kMaxLocals = 16;
  static constexpr size_t kMaxUpvalues = 16;

public:
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

  uint32_t GetLocalCount() const
  {
    return localCount_;
  }

  void SetLocalCount( uint32_t localCount )
  {
    if ( localCount > kMaxLocals )
      throw CompilerError( std::format( "Can't exceed more than {} local variables", kMaxLocals ) );
    localCount_ = static_cast<uint8_t>( localCount );
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

    // Check for duplicates in reverse order
    // TODO prefer inplace_vector for locals_, and then reverse iteration
    for (int i = localCount_ - 1; i >= 0; --i)
    {
      const Local& local = locals_[ i ];
      if ( local.IsInitialized() && local.GetDepth() < scopeDepth_ )
        break;
      if ( token.GetValue() == local.GetToken().GetValue() )
        throw CompilerError( "Already a variable with this name in scope" );
    }

#pragma warning(push)
#pragma warning(disable : 6385) // warning: buffer might be smaller than index -- but we've already checked
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
      const Local& local = locals_[ i ];
      if (identifierName == local.GetToken().GetValue())
      {
        if ( !local.IsInitialized() )
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
      if (upValues_[ i ].GetIndex() == index && upValues_[i].IsLocal() == isLocal )
      {
        index = i;
        return;
      }
    }

    // New upvalue
    //upValues_.emplace_back( { isLocal, index } ); TODO
    upValues_[ upvalueCount ] = Upvalue{ isLocal, index };
    index = upvalueCount;
    function_.IncrementUpvalueCount(); // TODO do we need to store this? Use upValues_.size()?
  }

  Upvalue GetUpvalue( uint32_t index ) const
  {
    if ( index >= FunctionInfo::kMaxUpvalues )
      throw CompilerError( std::format( "Can't exceed {} upvalues", FunctionInfo::kMaxUpvalues ) );
    return upValues_[ index ];
  }

  uint32_t GetScopeDepth() const
  {
    return scopeDepth_;
  }

  void IncrementScopeDepth()
  {
    if ( scopeDepth_ == std::numeric_limits<uint8_t>::max() )
      throw CompilerError( std::format( "Can't exceed scope depth of {}", std::numeric_limits<uint8_t>::max() ) );
    ++scopeDepth_;
  }

  void DecrementScopeDepth()
  {
    assert( scopeDepth_ != 0 );
    --scopeDepth_;
  }

private:

  Function function_; // TODO unique_ptr? TODO Closure
  FunctionType functionType_ = FunctionType::Script; // TODO FunctionType::GlobalScope?
  std::array<Upvalue, FunctionInfo::kMaxUpvalues> upValues_; // TODO inplace_vector
  Local locals_[ FunctionInfo::kMaxLocals ]; // TODO std::array or inplace_vector minimize size; 32?
  uint8_t localCount_ = 1; // compiler claims slot zero for the VM's internal use
  uint8_t scopeDepth_ = 0; // zero is global scope

}; // class FunctionInfo

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
