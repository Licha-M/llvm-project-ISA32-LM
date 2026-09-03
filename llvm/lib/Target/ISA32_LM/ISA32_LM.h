//===-- ISA32_LM.h - Top-level interface for ISA32_LM Target ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Declaraciones principales de funciones y componentes del target ISA32_LM.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ISA32_LM_ISA32_LM_H
#define LLVM_LIB_TARGET_ISA32_LM_ISA32_LM_H

#include "llvm/Support/CodeGen.h"



namespace llvm {
class FunctionPass;
class ISA32_LMTargetMachine;
class PassRegistry;

// TODO(fase 2): Función de creación del selector de instrucciones SelectionDAG.
FunctionPass *createISA32_LMISelDagLegacyPass(ISA32_LMTargetMachine &TM,
                                               CodeGenOptLevel OptLevel);

} // namespace llvm

#endif // LLVM_LIB_TARGET_ISA32_LM_ISA32_LM_H
