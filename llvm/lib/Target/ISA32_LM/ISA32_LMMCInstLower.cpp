//===-- ISA32_LMMCInstLower.cpp - Convert MachineInstr to MCInst ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implementación de la clase ISA32_LMMCInstLower.
//
//===----------------------------------------------------------------------===//

#include "ISA32_LMMCInstLower.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/ErrorHandling.h"

#define GET_REGINFO_ENUM
#include "ISA32_LMGenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#include "ISA32_LMGenInstrInfo.inc"
using namespace llvm;

const MCExpr *
ISA32_LMMCInstLower::lowerOperandExpr(const MachineOperand &MO) const {
  const MCSymbol *Sym = nullptr;

  switch (MO.getType()) {
  case MachineOperand::MO_MachineBasicBlock:
    Sym = MO.getMBB()->getSymbol();
    break;
  case MachineOperand::MO_GlobalAddress:
    Sym = Printer.getSymbol(MO.getGlobal());
    break;
  case MachineOperand::MO_ExternalSymbol:
    Sym = Printer.GetExternalSymbolSymbol(MO.getSymbolName());
    break;
  case MachineOperand::MO_BlockAddress:
    Sym = Printer.GetBlockAddressSymbol(MO.getBlockAddress());
    break;
  default:
    llvm_unreachable("Tipo de operando simbólico no soportado");
  }

  const MCExpr *Expr = MCSymbolRefExpr::create(Sym, Ctx);
  if (MO.getType() != MachineOperand::MO_MachineBasicBlock &&
      MO.getOffset() != 0)
    Expr = MCBinaryExpr::createAdd(
        Expr, MCConstantExpr::create(MO.getOffset(), Ctx), Ctx);

  return Expr;
}

void ISA32_LMMCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI) const {
  OutMI.setOpcode(MI->getOpcode());

  for (const MachineOperand &MO : MI->operands()) {
    MCOperand MCOp;
    switch (MO.getType()) {
    case MachineOperand::MO_Register:
      if (MO.isImplicit())
        continue;
      MCOp = MCOperand::createReg(MO.getReg());
      break;
    case MachineOperand::MO_Immediate:
      MCOp = MCOperand::createImm(MO.getImm());
      break;
    case MachineOperand::MO_MachineBasicBlock:
    case MachineOperand::MO_GlobalAddress:
    case MachineOperand::MO_ExternalSymbol:
    case MachineOperand::MO_BlockAddress:
      MCOp = MCOperand::createExpr(lowerOperandExpr(MO));
      break;
    case MachineOperand::MO_RegisterMask:
      continue;
    default:
      llvm_unreachable("Tipo de operando no soportado en Lower");
    }

    OutMI.addOperand(MCOp);
  }
}
