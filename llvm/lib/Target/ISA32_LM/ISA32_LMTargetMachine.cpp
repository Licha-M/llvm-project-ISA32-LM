//===-- ISA32_LMTargetMachine.cpp - TargetMachine for ISA32_LM ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implementación de ISA32_LMTargetMachine y registro del target.
//
//===----------------------------------------------------------------------===//

#include "ISA32_LMTargetMachine.h"
#include "ISA32_LM.h"
#include "TargetInfo/ISA32_LMTargetInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"

using namespace llvm;

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void LLVMInitializeISA32_LMTarget() {
  RegisterTargetMachine<ISA32_LMTargetMachine> X(getTheISA32_LMTarget());
}

static std::string computeDataLayout() {
  // DataLayout para ISA32_LM:
  //   e: Little-Endian
  //   m:e: ELF mangling
  //   p:32:32: Punteros de 32 bits alineados a 32 bits
  //   i64:32: Enteros de 64 bits alineados a 32 bits (ABI)
  //   n32: Ancho nativo de entero 32 bits
  //   S32: Alineación natural de pila 32 bits (4 bytes)
  return "e-m:e-p:32:32-i64:32-n32-S32";
}

static Reloc::Model getEffectiveRelocModel(std::optional<Reloc::Model> RM) {
  return RM.value_or(Reloc::Static);
}

ISA32_LMTargetMachine::ISA32_LMTargetMachine(
    const Target &T, const Triple &TT, StringRef Cpu, StringRef FeatureString,
    const TargetOptions &Options, std::optional<Reloc::Model> RM,
    std::optional<CodeModel::Model> CodeModel, CodeGenOptLevel OptLevel,
    bool JIT)
    : CodeGenTargetMachineImpl(
          T, computeDataLayout(), TT, Cpu, FeatureString, Options,
          getEffectiveRelocModel(RM),
          getEffectiveCodeModel(CodeModel, CodeModel::Small), OptLevel),
      Subtarget(TT, Cpu, FeatureString, *this),
      TLOF(std::make_unique<TargetLoweringObjectFileELF>()) {
  initAsmInfo();
}

namespace {
class ISA32_LMPassConfig : public TargetPassConfig {
public:
  ISA32_LMPassConfig(ISA32_LMTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  ISA32_LMTargetMachine &getISA32_LMTargetMachine() const {
    return getTM<ISA32_LMTargetMachine>();
  }

  bool addInstSelector() override {
    addPass(createISA32_LMISelDagLegacyPass(getISA32_LMTargetMachine(), getOptLevel()));
    return false;
  }
};
} // namespace

TargetPassConfig *
ISA32_LMTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new ISA32_LMPassConfig(*this, PM);
}
