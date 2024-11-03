///////////////////////////////////////////////////////////////////////////////
//
//  VirtualMachine.h
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
#include <cstdint>

namespace PKIsensee
{

class Chunk;

class VirtualMachine
{
public:
  VirtualMachine() = default;

  void Interpret( const Chunk* );

  // Disable copy/move
  VirtualMachine( const VirtualMachine& ) = delete;
  VirtualMachine& operator=( const VirtualMachine& ) = delete;
  VirtualMachine( VirtualMachine&& ) = delete;
  VirtualMachine& operator=( VirtualMachine&& ) = delete;

private:

  uint8_t ReadByte();
  void Run();

private:
  const Chunk* chunk_ = nullptr;
  const uint8_t* ip_ = nullptr; // instruction pointer

};

} // namespace PKIsensee

///////////////////////////////////////////////////////////////////////////////
