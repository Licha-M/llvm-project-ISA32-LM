//===-- ISA32_LMMCTargetDesc.cpp - ISA32_LM Target Descriptions -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Registro de fábricas de la capa MC para el target ISA32_LM.
//
//===----------------------------------------------------------------------===//

#include "ISA32_LMMCTargetDesc.h"
#include "ISA32_LMInstPrinter.h"
#include "ISA32_LMMCAsmInfo.h"
#include "TargetInfo/ISA32_LMTargetInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"
#include "llvm/TargetParser/Triple.h"
#include <string>

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "ISA32_LMGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "ISA32_LMGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "ISA32_LMGenRegisterInfo.inc"

using namespace llvm;

static MCInstrInfo *createISA32_LMMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitISA32_LMMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createISA32_LMMCRegisterInfo(const Triple & /*TT*/) {
  MCRegisterInfo *X = new MCRegisterInfo();
  // R14 es el Stack Pointer (link register para InitISA32_LMMCRegisterInfo)
  InitISA32_LMMCRegisterInfo(X, ISA32_LM::R14);
  return X;
}

static MCSubtargetInfo *
createISA32_LMMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  std::string CPUName = std::string(CPU);
  if (CPUName.empty())
    CPUName = "generic";
  return createISA32_LMMCSubtargetInfoImpl(TT, CPUName, /*TuneCPU=*/CPUName, FS);
}

static MCInstPrinter *createISA32_LMMCInstPrinter(const Triple & /*T*/,
                                                   unsigned SyntaxVariant,
                                                   const MCAsmInfo &MAI,
                                                   const MCInstrInfo &MII,
                                                   const MCRegisterInfo &MRI) {
  if (SyntaxVariant == 0)
    return new ISA32_LMInstPrinter(MAI, MII, MRI);
  return nullptr;
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeISA32_LMTargetMC() {
  Target &T = getTheISA32_LMTarget();

  // Registrar información de ensamblador (MCAsmInfo)
  RegisterMCAsmInfo<ISA32_LMMCAsmInfo> X(T);

  // Registrar información de instrucciones
  TargetRegistry::RegisterMCInstrInfo(T, createISA32_LMMCInstrInfo);

  // Registrar información de registros
  TargetRegistry::RegisterMCRegInfo(T, createISA32_LMMCRegisterInfo);

  // Registrar información de subtarget
  TargetRegistry::RegisterMCSubtargetInfo(T, createISA32_LMMCSubtargetInfo);

  // Registrar el InstPrinter
  TargetRegistry::RegisterMCInstPrinter(T, createISA32_LMMCInstPrinter);
}
