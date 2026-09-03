//===-- ISA32_LMInstrInfo.cpp - ISA32_LM Instruction Information ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implementación de la clase ISA32_LMInstrInfo.
//
//===----------------------------------------------------------------------===//

#include "ISA32_LMInstrInfo.h"
#include "ISA32_LM.h"
#include "ISA32_LMSubtarget.h"
#include "MCTargetDesc/ISA32_LMMCTargetDesc.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/ErrorHandling.h"

#define GET_INSTRINFO_CTOR_DTOR
#include "ISA32_LMGenInstrInfo.inc"

using namespace llvm;

ISA32_LMInstrInfo::ISA32_LMInstrInfo(const ISA32_LMSubtarget &STI)
    : ISA32_LMGenInstrInfo(STI, RegisterInfo, ISA32_LM::ADJCALLSTACKDOWN,
                           ISA32_LM::ADJCALLSTACKUP) {}

void ISA32_LMInstrInfo::copyPhysReg(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator Position,
    const DebugLoc &DL, Register DestinationRegister, Register SourceRegister,
    bool KillSource, bool /*RenamableDest*/, bool /*RenamableSrc*/) const {
  if (ISA32_LM::GPR32RegClass.contains(DestinationRegister, SourceRegister)) {
    // Usamos SLT_ADD_RRR (variante silenciosa de ADD) para copiar
    // SourceRegister -> DestinationRegister mediante SourceRegister + R0 ->
    // DestinationRegister.
    BuildMI(MBB, Position, DL, get(ISA32_LM::SLT_ADD_RRR), DestinationRegister)
        .addReg(SourceRegister, getKillRegState(KillSource))
        .addReg(ISA32_LM::R0);
    return;
  }

  if (ISA32_LM::SREGRegClass.contains(DestinationRegister) &&
      ISA32_LM::GPR32RegClass.contains(SourceRegister)) {
    // Copia de registro general a registro especial: CYR GPR, SREG
    BuildMI(MBB, Position, DL, get(ISA32_LM::CYR), DestinationRegister)
        .addReg(SourceRegister, getKillRegState(KillSource));
    return;
  }

  if (ISA32_LM::GPR32RegClass.contains(DestinationRegister) &&
      ISA32_LM::SREGRegClass.contains(SourceRegister)) {
    // Copia de registro especial a registro general: CYE SREG, GPR
    BuildMI(MBB, Position, DL, get(ISA32_LM::CYE), DestinationRegister)
        .addReg(SourceRegister, getKillRegState(KillSource));
    return;
  }

  llvm_unreachable("Copia de registros físicos no soportada para esta "
                   "combinación de registros");
}

void ISA32_LMInstrInfo::storeRegToStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator Position,
    Register SourceRegister, bool IsKill, int FrameIndex,
    const TargetRegisterClass *RegisterClass, Register VReg,
    MachineInstr::MIFlag Flags) const {
  DebugLoc DL;
  if (Position != MBB.end())
    DL = Position->getDebugLoc();

  BuildMI(MBB, Position, DL, get(ISA32_LM::STR_INT))
      .addFrameIndex(FrameIndex)
      .addReg(SourceRegister, getKillRegState(IsKill))
      .addImm(0)
      .setMIFlags(Flags);
}

void ISA32_LMInstrInfo::loadRegFromStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator Position,
    Register DestinationRegister, int FrameIndex,
    const TargetRegisterClass *RegisterClass, Register VReg, unsigned SubReg,
    MachineInstr::MIFlag Flags) const {
  DebugLoc DL;
  if (Position != MBB.end())
    DL = Position->getDebugLoc();

  BuildMI(MBB, Position, DL, get(ISA32_LM::LOD_INT), DestinationRegister)
      .addFrameIndex(FrameIndex)
      .addImm(0)
      .setMIFlags(Flags);
}

bool ISA32_LMInstrInfo::expandPostRAPseudo(MachineInstr &MI) const {
  MachineBasicBlock &MBB = *MI.getParent(); //[cite: 6]
  DebugLoc DL = MI.getDebugLoc();           //[cite: 6]

  switch (MI.getOpcode()) { //[cite: 6]
  default:                  //[cite: 6]
    return false;           //[cite: 6]

  case ISA32_LM::LDI32: {                        //[cite: 6]
    Register DstReg = MI.getOperand(0).getReg(); //[cite: 6]
    const MachineOperand &MO = MI.getOperand(1);

    if (MO.isImm()) {
      // Manejo de valores numéricos conocidos en tiempo de compilación
      int64_t Imm32 = MO.getImm(); //[cite: 6]

      uint32_t Low16 = Imm32 & 0xFFFF;              //[cite: 6]
      uint32_t High16Base = (Imm32 >> 16) & 0xFFFF; //[cite: 6]
      uint32_t High16Comp = (Low16 & 0x8000) ? ((High16Base + 1) & 0xFFFF)
                                             : High16Base; //[cite: 6]

      BuildMI(MBB, MI, DL, get(ISA32_LM::LDI), DstReg).addImm(0); //[cite: 6]
      BuildMI(MBB, MI, DL, get(ISA32_LM::HLDI), DstReg)
          .addImm(High16Comp);                             //[cite: 6]
      BuildMI(MBB, MI, DL, get(ISA32_LM::ADI_SLT), DstReg) //[cite: 6]
          .addReg(DstReg)                                  //[cite: 6]
          .addImm(Low16);                                  //[cite: 6]
    } else {
      // Manejo de símbolos (ConstantPoolIndex, GlobalAddress, etc.)
      BuildMI(MBB, MI, DL, get(ISA32_LM::LDI), DstReg).addImm(0);

      // Idealmente tu backend requerirá un TargetFlag indicando la reubicación
      // HI
      BuildMI(MBB, MI, DL, get(ISA32_LM::HLDI), DstReg).add(MO);

      // Idealmente tu backend requerirá un TargetFlag indicando la reubicación
      // LO
      BuildMI(MBB, MI, DL, get(ISA32_LM::ADI_SLT), DstReg)
          .addReg(DstReg)
          .add(MO);
    }

    MI.eraseFromParent(); //[cite: 6]
    return true;          //[cite: 6]
  }

  case ISA32_LM::JMP32_PSEUDO: //[cite: 6]
  case ISA32_LM::CAL32_PSEUDO: //[cite: 6]
  case ISA32_LM::BRH32_PSEUDO: //[cite: 6]
    return false;              //[cite: 6]
  }
}