//===-- ISA32_LMMCInstLower.h - Lower MachineInstr to MCInst ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Declaración de la clase ISA32_LMMCInstLower.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ISA32_LM_ISA32_LMMCINSTLOWER_H
#define LLVM_LIB_TARGET_ISA32_LM_ISA32_LMMCINSTLOWER_H

#include "llvm/Support/Compiler.h"

namespace llvm {
class AsmPrinter;
class MCContext;
class MCExpr;
class MCInst;
class MCOperand;
class MCSymbol;
class MachineInstr;
class MachineOperand;

class LLVM_LIBRARY_VISIBILITY ISA32_LMMCInstLower {
  MCContext &Ctx;
  AsmPrinter &Printer;

public:
  ISA32_LMMCInstLower(MCContext &CTX, AsmPrinter &AP) : Ctx(CTX), Printer(AP) {}

  void Lower(const MachineInstr *MI, MCInst &OutMI) const;
  const MCExpr *lowerOperandExpr(const MachineOperand &MO) const;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_ISA32_LM_ISA32_LMMCINSTLOWER_H
