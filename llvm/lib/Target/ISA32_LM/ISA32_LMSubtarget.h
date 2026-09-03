//===-- ISA32_LMSubtarget.h - ISA32_LM Subtarget Information ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Declaración de la clase ISA32_LMSubtarget.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ISA32_LM_ISA32_LMSUBTARGET_H
#define LLVM_LIB_TARGET_ISA32_LM_ISA32_LMSUBTARGET_H

#include "ISA32_LMFrameLowering.h"
#include "ISA32_LMISelLowering.h"
#include "ISA32_LMInstrInfo.h"
#include "ISA32_LMRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGTargetInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetMachine.h"

#define GET_SUBTARGETINFO_HEADER
#include "ISA32_LMGenSubtargetInfo.inc"

namespace llvm {

class ISA32_LMSubtarget : public ISA32_LMGenSubtargetInfo {
  ISA32_LMInstrInfo InstrInfo;
  ISA32_LMFrameLowering FrameLowering;
  ISA32_LMTargetLowering TLInfo;
  SelectionDAGTargetInfo TSInfo;

public:
  ISA32_LMSubtarget(const Triple &TT, StringRef CPU, StringRef FS,
                    const TargetMachine &TM);

  void ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS);

  ISA32_LMSubtarget &initializeSubtargetDependencies(StringRef CPU, StringRef FS);

  const ISA32_LMInstrInfo *getInstrInfo() const override {
    return &InstrInfo;
  }

  const ISA32_LMFrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }

  const ISA32_LMRegisterInfo *getRegisterInfo() const override {
    return &InstrInfo.getRegisterInfo();
  }

  const ISA32_LMTargetLowering *getTargetLowering() const override {
    return &TLInfo;
  }

  const SelectionDAGTargetInfo *getSelectionDAGInfo() const override {
    return &TSInfo;
  }
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_ISA32_LM_ISA32_LMSUBTARGET_H
