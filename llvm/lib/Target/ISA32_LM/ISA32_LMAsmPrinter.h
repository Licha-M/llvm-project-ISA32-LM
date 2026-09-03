//===-- ISA32_LMAsmPrinter.h - ISA32_LM Assembly Printer --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Declaración de la clase ISA32_LMAsmPrinter.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ISA32_LM_ISA32_LMASMPRINTER_H
#define LLVM_LIB_TARGET_ISA32_LM_ISA32_LMASMPRINTER_H

#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/Support/Compiler.h"

namespace llvm {

class ISA32_LMAsmPrinter : public AsmPrinter {
public:
  explicit ISA32_LMAsmPrinter(TargetMachine &TM,
                              std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)) {}

  StringRef getPassName() const override {
    return "ISA32_LM Assembly Printer";
  }

  void emitInstruction(const MachineInstr *MI) override;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_ISA32_LM_ISA32_LMASMPRINTER_H
