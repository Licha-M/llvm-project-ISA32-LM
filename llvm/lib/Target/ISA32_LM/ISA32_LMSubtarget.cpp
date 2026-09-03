//===-- ISA32_LMSubtarget.cpp - ISA32_LM Subtarget Information ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implementación de la clase ISA32_LMSubtarget.
//
//===----------------------------------------------------------------------===//

#include "ISA32_LMSubtarget.h"
#include "ISA32_LM.h"

#define DEBUG_TYPE "isa32_lm-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "ISA32_LMGenSubtargetInfo.inc"

using namespace llvm;

ISA32_LMSubtarget &
ISA32_LMSubtarget::initializeSubtargetDependencies(StringRef CPU, StringRef FS) {
  std::string CPUName = std::string(CPU);
  if (CPUName.empty())
    CPUName = "generic";

  ParseSubtargetFeatures(CPUName, /*TuneCPU=*/CPUName, FS);
  return *this;
}

ISA32_LMSubtarget::ISA32_LMSubtarget(const Triple &TT, StringRef CPU,
                                     StringRef FS, const TargetMachine &TM)
    : ISA32_LMGenSubtargetInfo(TT, CPU, /*TuneCPU=*/CPU, FS),
      InstrInfo(initializeSubtargetDependencies(CPU, FS)),
      FrameLowering(*this),
      TLInfo(TM, *this),
      TSInfo() {}
