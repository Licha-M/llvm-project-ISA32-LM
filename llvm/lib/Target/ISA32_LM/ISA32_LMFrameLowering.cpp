//===-- ISA32_LMFrameLowering.cpp - ISA32_LM Frame Lowering --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implementación de la clase ISA32_LMFrameLowering.
//
//===----------------------------------------------------------------------===//

#include "ISA32_LMFrameLowering.h"
#include "ISA32_LM.h"
#include "ISA32_LMRegisterInfo.h"
#include "ISA32_LMSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/MathExtras.h"

#define GET_REGINFO_ENUM
#include "ISA32_LMGenRegisterInfo.inc"
#define GET_INSTRINFO_ENUM
#include "ISA32_LMGenInstrInfo.inc"
#include "ISA32_LMGenCallingConv.inc"

using namespace llvm;

StackOffset ISA32_LMFrameLowering::getFrameIndexReference(
    const MachineFunction &MF, int FI, Register &FrameReg) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  FrameReg = ISA32_LM::R14; // Siempre relativo al Stack Pointer (R14)
  return StackOffset::getFixed(MFI.getObjectOffset(FI));
}

MachineBasicBlock::iterator ISA32_LMFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction & /*MF*/, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator I) const {
  // Como los argumentos salientes están reservados estáticamente en la pila
  // (hasReservedCallFrame = true), ADJCALLSTACKDOWN y ADJCALLSTACKUP no emiten
  // código y simplemente se eliminan del basic block.
  return MBB.erase(I);
}

void ISA32_LMFrameLowering::emitPrologue(MachineFunction &MF,
                                         MachineBasicBlock &MBB) const {
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();

  uint64_t StackSize = alignTo(MFI.getStackSize(), getStackAlign());
  MFI.setStackSize(StackSize);

  if (StackSize == 0)
    return;

  // En ISA32_LM la pila crece hacia ARRIBA (StackGrowsUp / Low to High).
  // El prólogo incrementa R14 en FrameSize.
  if (isInt<16>(StackSize)) {
    // Si FrameSize cabe en 16 bits signed, usamos ADI directamente
    BuildMI(MBB, MBBI, DL, TII.get(ISA32_LM::ADI), ISA32_LM::R14)
        .addReg(ISA32_LM::R14)
        .addImm(StackSize);
  } else {
    // Para frames mayores a 16 bits signed, cargamos el tamaño en R15 (scratch)
    // y sumamos R14 += R15 mediante ADD_RRR.
    BuildMI(MBB, MBBI, DL, TII.get(ISA32_LM::LDI32), ISA32_LM::R15)
        .addImm(StackSize);
    BuildMI(MBB, MBBI, DL, TII.get(ISA32_LM::ADD_RRR), ISA32_LM::R14)
        .addReg(ISA32_LM::R14)
        .addReg(ISA32_LM::R15);
  }
}

void ISA32_LMFrameLowering::emitEpilogue(MachineFunction &MF,
                                         MachineBasicBlock &MBB) const {
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();

  uint64_t StackSize = MFI.getStackSize();
  if (StackSize == 0)
    return;

  // Para liberar la pila al retornar, restamos FrameSize a R14.
  int64_t NegStackSize = -static_cast<int64_t>(StackSize);

  if (isInt<16>(NegStackSize)) {
    BuildMI(MBB, MBBI, DL, TII.get(ISA32_LM::ADI), ISA32_LM::R14)
        .addReg(ISA32_LM::R14)
        .addImm(NegStackSize);
  } else {
    BuildMI(MBB, MBBI, DL, TII.get(ISA32_LM::LDI32), ISA32_LM::R15)
        .addImm(NegStackSize);
    BuildMI(MBB, MBBI, DL, TII.get(ISA32_LM::ADD_RRR), ISA32_LM::R14)
        .addReg(ISA32_LM::R14)
        .addReg(ISA32_LM::R15);
  }
}
