//===-- ISA32_LMFrameLowering.h - Define frame lowering for ISA32_LM --*- C++
//-*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Declaración de la clase ISA32_LMFrameLowering.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ISA32_LM_ISA32_LMFRAMELOWERING_H
#define LLVM_LIB_TARGET_ISA32_LM_ISA32_LMFRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {

class ISA32_LMSubtarget;

class ISA32_LMFrameLowering : public TargetFrameLowering {
protected:
  const ISA32_LMSubtarget &STI;

public:
  explicit ISA32_LMFrameLowering(const ISA32_LMSubtarget &Subtarget)
      : TargetFrameLowering(
            StackGrowsUp,
            /*StackAlignment=*/Align(4),
            // LocalAreaOffset=4: reserva el slot en SP_old+0 para el PC
            // que la instrucción CAL del hijo escribe automáticamente
            // (CAL guarda PC en RAM[SP] y luego hace SP += 4).
            // El primer byte usable para variables/registros es SP_old+4.
            /*LocalAreaOffset=*/4),
        STI(Subtarget) {}

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I) const override;

  bool hasReservedCallFrame(const MachineFunction &MF) const override {
    return true;
  }

  StackOffset getFrameIndexReference(const MachineFunction &MF, int FI,
                                     Register &FrameReg) const override;

protected:
  bool hasFPImpl(const MachineFunction &MF) const override { return false; }
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_ISA32_LM_ISA32_LMFRAMELOWERING_H
