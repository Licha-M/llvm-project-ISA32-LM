//===-- ISA32_LMRegisterInfo.h - ISA32_LM Register Info Iface ---*- C++ -*-===//
//
// Interfaz de RegisterInfo para la arquitectura ISA32_LM.
//
// Política de registros reservados (getReservedRegs):
//   R0  — constante 0 (wire-zero); el hardware ignora escrituras.
//   R14 — Stack Pointer (SP) autónomo por hardware; gestionado por
//          FrameLowering, nunca por el allocator genérico.
//   R15 — Scratch exclusivo del ensamblador y de la expansión de saltos
//          de 32 bits; LLVM nunca lo asigna como registro de propósito general.
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ISA32_LM_ISA32_LMREGISTERINFO_H
#define LLVM_LIB_TARGET_ISA32_LM_ISA32_LMREGISTERINFO_H

#include "llvm/CodeGen/TargetRegisterInfo.h"

// Archivo generado por TableGen a partir de ISA32_LMRegisterInfo.td.
// Contiene: enumeraciones de registros, descriptores de RegisterClass, etc.
#define GET_REGINFO_HEADER
#include "ISA32_LMGenRegisterInfo.inc"

namespace llvm {

class ISA32_LMRegisterInfo : public ISA32_LMGenRegisterInfo {
public:
  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;
  const uint32_t *getCallPreservedMask(const MachineFunction &MF, CallingConv::ID CC) const override;


  ISA32_LMRegisterInfo();

  //--------------------------------------------------------------------------
  // Registros reservados
  //
  // Devuelve un BitVector con un bit a 1 por cada registro que el allocator
  // de registros NO puede asignar libremente.
  //
  // Siempre reservados:
  //   R0  — constante 0; escribir en él no tiene efecto en el hardware.
  //   R14 — Stack Pointer (SP); gestionado exclusivamente por FrameLowering.
  //   R15 — Scratch del ensamblador para expansión de saltos de 32 bits;
  //          LLVM nunca lo debe asignar como GPR de propósito general.
  //--------------------------------------------------------------------------
  BitVector getReservedRegs(const MachineFunction &MF) const override;

  //--------------------------------------------------------------------------
  // Eliminación del frame index (resolución de referencias al stack frame)
  //
  // Convierte los MachineOperand de tipo FrameIndex en referencias concretas
  // de registro + offset, usando R14 (SP) como base del marco.
  //--------------------------------------------------------------------------
  bool eliminateFrameIndex(MachineBasicBlock::iterator MI,
                           int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  //--------------------------------------------------------------------------
  // Registro de frame pointer del target
  //
  // ISA32_LM utiliza R14 como Stack Pointer (y frame pointer, dado que no
  // existe un FP dedicado). Devuelve ISA32_LM::R14.
  //--------------------------------------------------------------------------
  Register getFrameRegister(const MachineFunction &MF) const override;

  //--------------------------------------------------------------------------
  // Registros preservados a través de llamadas (callee-saved registers)
  //
  // Según la convención de llamada ISA32_LM:
  //   Preservados (callee-saved): R8, R9, R10, R11, R12, R13
  //   Volátiles   (caller-saved): R1,  R2,  R3,  R4,  R5,  R6, R7
  //   Reservados:                 R0 (zero), R14 (SP), R15 (scratch)
  //
  // TableGen genera getCalleeSavedRegs() a partir de la definición de
  // CallingConv (se añadirá en ISA32_LMCallingConv.td).
  // El método aquí hereda la implementación de ISA32_LMGenRegisterInfo.
  //--------------------------------------------------------------------------
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_ISA32_LM_ISA32_LMREGISTERINFO_H
