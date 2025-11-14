///////////////////////////////////////////////////////////////////////////////
//
//  Function.cpp
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

#include "ByteCodeBlock.h"
#include "CompilerError.h"
#include "Function.h"

using namespace PKIsensee;

Function::Function() :
  byteCodeBlock_{ std::make_shared<ByteCodeBlock>() }
{
}

///////////////////////////////////////////////////////////////////////////////
