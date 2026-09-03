//===-- ISA32_LMAsmPrinter.cpp - ISA32_LM Assembly Printer ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implementación de la clase ISA32_LMAsmPrinter.
//
//===----------------------------------------------------------------------===//

#include "ISA32_LMAsmPrinter.h"
#include "ISA32_LMMCInstLower.h"
#include "MCTargetDesc/ISA32_LMMCAsmInfo.h"
#include "MCTargetDesc/ISA32_LMMCTargetDesc.h"
#include "TargetInfo/ISA32_LMTargetInfo.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"

using namespace llvm;

#define DEBUG_TYPE "asm-printer"

void ISA32_LMAsmPrinter::emitInstruction(const MachineInstr *MI) {
  ISA32_LMMCInstLower MCInstLowering(OutContext, *this);
  MCSubtargetInfo STI = getSubtargetInfo();

  switch (MI->getOpcode()) {
  case ISA32_LM::JMP32_PSEUDO: {
    const MachineOperand &MO = MI->getOperand(0);
    const MCExpr *Expr = MCInstLowering.lowerOperandExpr(MO);
    const MCExpr *ExprHi =
        MCSpecifierExpr::create(Expr, ISA32_LM::S_HI16, OutContext);
    const MCExpr *ExprLo =
        MCSpecifierExpr::create(Expr, ISA32_LM::S_LO16, OutContext);

    OutStreamer->emitInstruction(
        MCInstBuilder(ISA32_LM::HLDI).addReg(ISA32_LM::R15).addExpr(ExprHi),
        STI);
    OutStreamer->emitInstruction(MCInstBuilder(ISA32_LM::ADI_SLT)
                                     .addReg(ISA32_LM::R15)
                                     .addReg(ISA32_LM::R15)
                                     .addExpr(ExprLo),
                                 STI);
    OutStreamer->emitInstruction(
        MCInstBuilder(ISA32_LM::JMP).addReg(ISA32_LM::R15), STI);
    return;
  }

  case ISA32_LM::CAL32_PSEUDO: {
    const MachineOperand &MO = MI->getOperand(0);
    const MCExpr *Expr = MCInstLowering.lowerOperandExpr(MO);
    const MCExpr *ExprHi =
        MCSpecifierExpr::create(Expr, ISA32_LM::S_HI16, OutContext);
    const MCExpr *ExprLo =
        MCSpecifierExpr::create(Expr, ISA32_LM::S_LO16, OutContext);

    OutStreamer->emitInstruction(
        MCInstBuilder(ISA32_LM::HLDI).addReg(ISA32_LM::R15).addExpr(ExprHi),
        STI);
    OutStreamer->emitInstruction(MCInstBuilder(ISA32_LM::ADI_SLT)
                                     .addReg(ISA32_LM::R15)
                                     .addReg(ISA32_LM::R15)
                                     .addExpr(ExprLo),
                                 STI);
    OutStreamer->emitInstruction(
        MCInstBuilder(ISA32_LM::CAL).addReg(ISA32_LM::R15), STI);
    return;
  }

  case ISA32_LM::BRH32_PSEUDO: {
    // 1. Imprimir la instruccion en la consola para depuracion
    errs() << "DEBUG BRH32: ";
    MI->print(errs());
    errs() << "\n";

    Register LHS = MI->getOperand(0).getReg();
    Register RHS = MI->getOperand(1).getReg();

    // 2. Auto-deteccion de los indices
    int idxCC = 2;
    int idxTarget = 3;

    // Si el operando 2 es el Bloque Basico (etiqueta) y no el Inmediato, los
    // invertimos
    if (MI->getOperand(2).isMBB() || !MI->getOperand(2).isImm()) {
      idxCC = 3;
      idxTarget = 2;
    }

    // 3. Extraer los valores con los indices ya corregidos
    int64_t CC = MI->getOperand(idxCC).getImm();
    const MachineOperand &TargetMO = MI->getOperand(idxTarget);

    // 4. Resolver la etiqueta
    const MCExpr *Expr = nullptr;
    if (TargetMO.isMBB()) {
      Expr =
          MCSymbolRefExpr::create(TargetMO.getMBB()->getSymbol(), OutContext);
    } else {
      Expr = MCInstLowering.lowerOperandExpr(TargetMO);
    }

    const MCExpr *ExprHi =
        MCSpecifierExpr::create(Expr, ISA32_LM::S_HI16, OutContext);
    const MCExpr *ExprLo =
        MCSpecifierExpr::create(Expr, ISA32_LM::S_LO16, OutContext);

    // 5. Emitir instrucciones nativas
    OutStreamer->emitInstruction(MCInstBuilder(ISA32_LM::SUB_RRR)
                                     .addReg(ISA32_LM::R0)
                                     .addReg(LHS)
                                     .addReg(RHS),
                                 STI);
    OutStreamer->emitInstruction(
        MCInstBuilder(ISA32_LM::HLDI).addReg(ISA32_LM::R15).addExpr(ExprHi),
        STI);
    OutStreamer->emitInstruction(MCInstBuilder(ISA32_LM::ADI_SLT)
                                     .addReg(ISA32_LM::R15)
                                     .addReg(ISA32_LM::R15)
                                     .addExpr(ExprLo),
                                 STI);
    OutStreamer->emitInstruction(
        MCInstBuilder(ISA32_LM::BRH).addImm(CC).addReg(ISA32_LM::R15), STI);

    return;
  }
  }

  MCInst TmpInst;
  MCInstLowering.Lower(MI, TmpInst);
  OutStreamer->emitInstruction(TmpInst, STI);
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeISA32_LMAsmPrinter() {
  RegisterAsmPrinter<ISA32_LMAsmPrinter> X(getTheISA32_LMTarget());
}
