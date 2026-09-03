//===-- ISA32_LMTargetMachine.h - TargetMachine for ISA32_LM ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Declaración de la clase ISA32_LMTargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ISA32_LM_ISA32_LMTARGETMACHINE_H
#define LLVM_LIB_TARGET_ISA32_LM_ISA32_LMTARGETMACHINE_H

#include "ISA32_LMSubtarget.h"
#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include <optional>

namespace llvm {

class ISA32_LMTargetMachine : public CodeGenTargetMachineImpl {
  ISA32_LMSubtarget Subtarget;
  std::unique_ptr<TargetLoweringObjectFile> TLOF;

public:
  ISA32_LMTargetMachine(const Target &TheTarget, const Triple &TargetTriple,
                        StringRef Cpu, StringRef FeatureString,
                        const TargetOptions &Options,
                        std::optional<Reloc::Model> RM,
                        std::optional<CodeModel::Model> CodeModel,
                        CodeGenOptLevel OptLevel, bool JIT);

  const ISA32_LMSubtarget *
  getSubtargetImpl(const Function & /*Fn*/) const override {
    return &Subtarget;
  }

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_ISA32_LM_ISA32_LMTARGETMACHINE_H
