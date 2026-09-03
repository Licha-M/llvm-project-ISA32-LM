//===-- ISA32_LMMCAsmInfo.h - ISA32_LM MCAsmInfo ----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Declaración de la clase ISA32_LMMCAsmInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ISA32_LM_MCTARGETDESC_ISA32_LMMCASMINFO_H
#define LLVM_LIB_TARGET_ISA32_LM_MCTARGETDESC_ISA32_LMMCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {
class Triple;

class ISA32_LMMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit ISA32_LMMCAsmInfo(const Triple &TheTriple,
                              const MCTargetOptions &Options);

  void printSpecifierExpr(raw_ostream &OS,
                          const MCSpecifierExpr &Expr) const override;
};

namespace ISA32_LM {
using Specifier = uint8_t;
enum { S_None, S_HI16, S_LO16 };
} // namespace ISA32_LM

} // namespace llvm

#endif // LLVM_LIB_TARGET_ISA32_LM_MCTARGETDESC_ISA32_LMMCASMINFO_H
