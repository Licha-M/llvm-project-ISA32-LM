//===-- ISA32_LMInstPrinter.cpp - Convert ISA32_LM MCInst to asm ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implementación del InstPrinter para ISA32_LM.
//
//===----------------------------------------------------------------------===//

#include "ISA32_LMInstPrinter.h"
#include "MCTargetDesc/ISA32_LMMCTargetDesc.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "asm-printer"

// Incluir el fragmento autogenerado por TableGen a partir de ISA32_LM.td.
// PRINT_ALIAS_INSTR activa la generación del método printAliasInstr().
#define PRINT_ALIAS_INSTR
#include "ISA32_LMGenAsmWriter.inc"

void ISA32_LMInstPrinter::printRegName(raw_ostream &OS, MCRegister Reg) {
  // Los registros se imprimen en mayúsculas sin prefijo: R0..R15, SR0..SR15.
  OS << getRegisterName(Reg);
}

void ISA32_LMInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                    StringRef Annotation,
                                    const MCSubtargetInfo & /*STI*/,
                                    raw_ostream &O) {
  // Intentar primero los alias (p.ej. CMP como alias de SUB_RRR con R0 como Rd).
  if (!printAliasInstr(MI, Address, O))
    printInstruction(MI, Address, O);
  printAnnotation(O, Annotation);
}

void ISA32_LMInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                       raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isReg()) {
    // Sin prefijo — la sintaxis de ISA32_LM usa "R0", no "%R0" ni "$R0".
    O << getRegisterName(Op.getReg());
  } else if (Op.isImm()) {
    O << Op.getImm();
  } else {
    assert(Op.isExpr() && "Se esperaba una expresión");
    MAI.printExpr(O, *Op.getExpr());
  }
}

// printImm16Operand — operando uimm16 (LDI, HLDI): sin signo, decimal.
void ISA32_LMInstPrinter::printImm16Operand(const MCInst *MI, unsigned OpNo,
                                             raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isImm()) {
    // Mascarar a 16 bits sin signo para que valores negativos se vean correctamente.
    O << (Op.getImm() & 0xFFFF);
  } else {
    assert(Op.isExpr() && "Se esperaba una expresión");
    MAI.printExpr(O, *Op.getExpr());
  }
}

// printSImm16Operand — operando simm16 (ADI, ADI_SLT, offsets LOD/STR): con signo, decimal.
void ISA32_LMInstPrinter::printSImm16Operand(const MCInst *MI, unsigned OpNo,
                                              raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isImm()) {
    // Extender el signo de 16 bits y mostrar como decimal con signo.
    int16_t Imm = static_cast<int16_t>(Op.getImm() & 0xFFFF);
    O << static_cast<int32_t>(Imm);
  } else {
    assert(Op.isExpr() && "Se esperaba una expresión");
    MAI.printExpr(O, *Op.getExpr());
  }
}

// printBRHCondOperand — código de condición de BRH (4 bits) → mnemónico.
// Tabla de condiciones (de ISA32_LMInstrInfo.td y translateISA32LMCC):
//   0=EQ  1=NE  2=N  3=NN  4=C  5=NC  6=OV  7=NOV
void ISA32_LMInstPrinter::printBRHCondOperand(const MCInst *MI, unsigned OpNo,
                                               raw_ostream &O) {
  static const char *const CondNames[] = {
      "EQ", "NE", "N", "NN", "C", "NC", "OV", "NOV"};
  int64_t CC = MI->getOperand(OpNo).getImm();
  if (CC >= 0 && CC < 8)
    O << CondNames[CC];
  else
    O << "<cond:" << CC << ">";
}

// printBrTargetOperand — destino simbólico de un salto (referencia a BB o símbolo global).
void ISA32_LMInstPrinter::printBrTargetOperand(const MCInst *MI, unsigned OpNo,
                                                raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isExpr()) {
    MAI.printExpr(O, *Op.getExpr());
  } else if (Op.isImm()) {
    O << Op.getImm();
  } else {
    assert(false && "Operando de destino de salto no esperado");
  }
}
