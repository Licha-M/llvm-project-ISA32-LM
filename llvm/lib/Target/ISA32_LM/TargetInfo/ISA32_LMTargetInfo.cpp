//===-- ISA32_LMTargetInfo.cpp - ISA32_LM Target Implementation -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/ISA32_LMTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"

using namespace llvm;

Target &llvm::getTheISA32_LMTarget() {
  static Target TheISA32_LMTarget;
  return TheISA32_LMTarget;
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeISA32_LMTargetInfo() {
  RegisterTarget<Triple::UnknownArch> X(
      getTheISA32_LMTarget(), "isa32_lm", "ISA32_LM (Custom 32-bit RISC)",
      "ISA32_LM");
}
