//===-- ISA32_LMRegisterInfo.cpp - ISA32_LM Register Information ----------===//
//
// Implementación de RegisterInfo para la arquitectura ISA32_LM.
//
// Arquitectura: RISC custom de 32 bits, Little-Endian.
//
// Banco de registros general (GPR):
//   R0–R15 (32 bits, sin sub-registros).
//
// Convención de llamada:
//   Volátiles   (caller-saved): R1–R7
//   Preservados (callee-saved): R8–R13
//   Reservados:  R0 (=0), R14 (SP), R15 (scratch ensamblador)
//===----------------------------------------------------------------------===//

#include "ISA32_LMRegisterInfo.h"
#include "ISA32_LM.h"          // Namespace y constantes del target
#include "ISA32_LMSubtarget.h" // Información del subtarget
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/ErrorHandling.h"

#define GET_REGINFO_ENUM
#include "ISA32_LMGenRegisterInfo.inc"

#define GET_REGINFO_TARGET_DESC
#include "ISA32_LMGenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#include "ISA32_LMGenInstrInfo.inc"

using namespace llvm;

//===----------------------------------------------------------------------===//
// Constructor
//===----------------------------------------------------------------------===//

ISA32_LMRegisterInfo::ISA32_LMRegisterInfo()
    // El primer argumento es el registro de dirección de retorno.
    // En ISA32_LM el hardware almacena la dirección de retorno internamente
    // (no existe un registro de enlace visible), por lo que se pasa R0 como
    // placeholder. Ajusta según tu diseño si se expone un link register.
    : ISA32_LMGenRegisterInfo(ISA32_LM::R0) {}

//===----------------------------------------------------------------------===//
// getReservedRegs — Registros que el allocator nunca puede asignar
//===----------------------------------------------------------------------===//

BitVector
ISA32_LMRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  // Inicializa el BitVector con todos los registros físicos disponibles.
  BitVector Reserved(getNumRegs());

  // ── R0 — Constante cero (wire-zero) ────────────────────────────────────
  // El hardware ignora silenciosamente cualquier escritura en R0; siempre
  // lee 0. El allocator NUNCA debe asignarlo como destino de una operación.
  Reserved.set(ISA32_LM::R0);

  // ── R14 — Stack Pointer (SP) ────────────────────────────────────────────
  // R14 es el puntero de pila autónomo por hardware. Su valor es mantenido
  // exclusivamente por FrameLowering (prólogo y epílogo de función) y por
  // las instrucciones de ajuste ADJCALLSTACKDOWN / ADJCALLSTACKUP.
  // El allocator genérico no debe asignarlo como GPR libre.
  Reserved.set(ISA32_LM::R14);

  // ── R15 — Scratch del ensamblador / cálculos de salto de 32 bits ────────
  // R15 está reservado estrictamente para:
  //   · La expansión de JMP32_PSEUDO → H LDI + SLT ADI + JMP R15
  //   · La expansión de CAL32_PSEUDO → H LDI + SLT ADI + CAL R15
  //   · La expansión de BRH32_PSEUDO → H LDI + SLT ADI + BRH COND, R15
  // LLVM nunca debe asignarlo como registro de propósito general; tratarlo
  // como un registro muerto desde el punto de vista del allocator.
  Reserved.set(ISA32_LM::R15);

  return Reserved;
}

//===----------------------------------------------------------------------===//
// eliminateFrameIndex — Resolución de FrameIndex a SP + offset
//===----------------------------------------------------------------------===//

bool ISA32_LMRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                               int SPAdj, unsigned FIOperandNum,
                                               RegScavenger *RS) const {

  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();
  const MachineFrameInfo &MFI = MF.getFrameInfo();

  // Obtiene el índice del frame y el offset del operando.
  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();

  // En ISA32_LM la pila crece hacia ARRIBA (StackGrowsUp).
  // Tras el prólogo, R14 apunta al TOPE del marco (R14 = SP_old + FrameSize).
  //
  // LLVM asigna ObjectOffset contando desde el FONDO del marco (SP_old),
  // con LocalAreaOffset=4 por lo que el primer slot usable está en SP_old+4.
  //
  // offset_real = ObjectOffset - FrameSize + SPAdj   →  NEGATIVO desde R14
  //
  // Ejemplo con FrameSize=20, ObjectOffset=4:
  //   Offset = 4 - 20 + 0 = -16   →  STR R14, Rx, -16  ✓
  // Ejemplo con FrameSize=20, ObjectOffset=16:
  //   Offset = 16 - 20 + 0 = -4   →  STR R14, Rx, -4   ✓
  int64_t FrameSize = static_cast<int64_t>(MFI.getStackSize());
  int64_t Offset = MFI.getObjectOffset(FrameIndex) - FrameSize + SPAdj;

  unsigned Opcode = MI.getOpcode();

  // CORRECCIÓN: Se agregó ADI_SLT para que se procese igual que ADI
  if (Opcode == ISA32_LM::ADI || Opcode == ISA32_LM::ADI_SLT) {
    // Para ADI R14, FrameIndex: el FrameIndex mismo en FIOperandNum es el
    // operando de offset
    MI.getOperand(FIOperandNum).ChangeToImmediate(Offset);
  } else if (Opcode == ISA32_LM::STR_CHAR || Opcode == ISA32_LM::STR_SHORT ||
             Opcode == ISA32_LM::STR_INT) {
    // En instrucciones STR_*, el registro base está en FIOperandNum y el offset
    // en FIOperandNum + 2
    MI.getOperand(FIOperandNum)
        .ChangeToRegister(ISA32_LM::R14, /*isDef=*/false);
    MI.getOperand(FIOperandNum + 2).ChangeToImmediate(Offset);
  } else {
    // Para LOD_* (y otras instrucciones base + offset): offset en FIOperandNum
    // + 1
    MI.getOperand(FIOperandNum)
        .ChangeToRegister(ISA32_LM::R14, /*isDef=*/false);

    // CORRECCIÓN: Chequeo de seguridad para evitar desbordamientos de arreglos
    // (Out of Bounds)
    if (FIOperandNum + 1 < MI.getNumOperands()) {
      MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);
    }
  }

  return false; // No se generaron nuevas instrucciones.
}

//===----------------------------------------------------------------------===//
// getFrameRegister — Registro de frame del target
//===----------------------------------------------------------------------===//

Register
ISA32_LMRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  // ISA32_LM usa R14 como Stack Pointer y, al no tener un frame pointer
  // dedicado, también como frame register.
  return ISA32_LM::R14;
}

const MCPhysReg *
ISA32_LMRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_ISA32_LM_SaveList;
}

const uint32_t *
ISA32_LMRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                           CallingConv::ID CC) const {
  return CSR_ISA32_LM_RegMask;
}