//===-- ISA32_LMMCTargetDesc.h - ISA32_LM Target Descriptions ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Declaraciones de la capa MC para el target ISA32_LM.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ISA32_LM_MCTARGETDESC_ISA32_LMMCTARGETDESC_H
#define LLVM_LIB_TARGET_ISA32_LM_MCTARGETDESC_ISA32_LMMCTARGETDESC_H

#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/Support/DataTypes.h"

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCSubtargetInfo;
class Target;
} // namespace llvm

// Enumeraciones de registros generadas por TableGen.
#define GET_REGINFO_ENUM
#include "ISA32_LMGenRegisterInfo.inc"

// Enumeraciones de instrucciones generadas por TableGen.
#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "ISA32_LMGenInstrInfo.inc"

// Enumeraciones de subtarget generadas por TableGen.
#define GET_SUBTARGETINFO_ENUM
#include "ISA32_LMGenSubtargetInfo.inc"

#endif // LLVM_LIB_TARGET_ISA32_LM_MCTARGETDESC_ISA32_LMMCTARGETDESC_H
