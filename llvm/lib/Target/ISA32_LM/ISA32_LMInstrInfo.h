//===-- ISA32_LMInstrInfo.h - ISA32_LM Instruction Information --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Declaración de la clase ISA32_LMInstrInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ISA32_LM_ISA32_LMINSTRINFO_H
#define LLVM_LIB_TARGET_ISA32_LM_ISA32_LMINSTRINFO_H

#include "ISA32_LMRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "ISA32_LMGenInstrInfo.inc"

namespace llvm {

class ISA32_LMSubtarget;

class ISA32_LMInstrInfo : public ISA32_LMGenInstrInfo {
  const ISA32_LMRegisterInfo RegisterInfo;

public:
  explicit ISA32_LMInstrInfo(const ISA32_LMSubtarget &STI);

  const ISA32_LMRegisterInfo &getRegisterInfo() const {
    return RegisterInfo;
  }

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator Position,
                   const DebugLoc &DL, Register DestinationRegister,
                   Register SourceRegister, bool KillSource,
                   bool RenamableDest = false,
                   bool RenamableSrc = false) const override;
// En ISA32_LMInstrInfo.h:
void storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI, 
                         Register SrcReg, bool isKill, int FrameIndex, 
                         const TargetRegisterClass *RC, Register VReg,
                         MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

void loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
                          Register DestReg, int FrameIndex, const TargetRegisterClass *RC,
                          Register VReg, unsigned SubReg = 0,
                          MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

  bool expandPostRAPseudo(MachineInstr &MI) const override;
};



} // namespace llvm

#endif // LLVM_LIB_TARGET_ISA32_LM_ISA32_LMINSTRINFO_H
