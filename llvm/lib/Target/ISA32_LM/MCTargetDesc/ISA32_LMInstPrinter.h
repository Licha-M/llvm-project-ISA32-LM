//===-- ISA32_LMInstPrinter.h - Convert ISA32_LM MCInst to asm ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Clase que convierte un MCInst de ISA32_LM en texto ensamblador.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ISA32_LM_MCTARGETDESC_ISA32_LMINSTPRINTER_H
#define LLVM_LIB_TARGET_ISA32_LM_MCTARGETDESC_ISA32_LMINSTPRINTER_H

#include "llvm/MC/MCInstPrinter.h"

namespace llvm {

class ISA32_LMInstPrinter : public MCInstPrinter {
public:
  ISA32_LMInstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                      const MCRegisterInfo &MRI)
      : MCInstPrinter(MAI, MII, MRI) {}

  // Punto de entrada principal para imprimir una instrucción.
  void printInst(const MCInst *MI, uint64_t Address, StringRef Annot,
                 const MCSubtargetInfo &STI, raw_ostream &O) override;

  // Impresión de un operando genérico (registro o inmediato sin PrintMethod custom).
  void printOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O);

  // --- Métodos custom de impresión referenciados desde ISA32_LMInstrInfo.td ---

  // uimm16 — usado por LDI / HLDI: valor sin signo de 16 bits en decimal.
  void printImm16Operand(const MCInst *MI, unsigned OpNo, raw_ostream &O);

  // simm16 — usado por ADI / ADI_SLT / offsets LOD / STR: valor con signo en decimal.
  void printSImm16Operand(const MCInst *MI, unsigned OpNo, raw_ostream &O);

  // brh_cond — código de condición de 4 bits de BRH: imprime el mnemónico ("EQ","NE",...).
  void printBRHCondOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O);

  // brh_target — referencia simbólica a un basic block de destino de un salto.
  void printBrTargetOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O);

  // --- Métodos autogenerados por TableGen ---
  std::pair<const char *, uint64_t>
  getMnemonic(const MCInst &MI) const override;
  void printInstruction(const MCInst *MI, uint64_t Address, raw_ostream &O);
  bool printAliasInstr(const MCInst *MI, uint64_t Address, raw_ostream &OS);
  void printCustomAliasOperand(const MCInst *MI, uint64_t Address,
                               unsigned OpIdx, unsigned PrintMethodIdx,
                               raw_ostream &O);
  static const char *getRegisterName(MCRegister Reg);
  void printRegName(raw_ostream &OS, MCRegister Reg) override;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_ISA32_LM_MCTARGETDESC_ISA32_LMINSTPRINTER_H
