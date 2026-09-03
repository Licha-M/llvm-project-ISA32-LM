//===-- ISA32_LMMCAsmInfo.cpp - ISA32_LM MCAsmInfo ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implementación de la clase ISA32_LMMCAsmInfo.
//
//===----------------------------------------------------------------------===//

#include "ISA32_LMMCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/Triple.h"

using namespace llvm;

void ISA32_LMMCAsmInfo::anchor() {}

ISA32_LMMCAsmInfo::ISA32_LMMCAsmInfo(const Triple & /*TheTriple*/,
                                     const MCTargetOptions &Options)
    : MCAsmInfoELF(Options) {
  // ISA32_LM es Little-Endian
  IsLittleEndian = true;

  // El comentario de línea en la sintaxis del ISA32_LM es ";"
  // Debe coincidir con CommentDelimiter = ";" ya declarado en ISA32_LM.td.
  CommentString = ";";

  // Alineación de instrucciones: ISA32_LM usa instrucciones de 32 bits alineadas a 4 bytes
  MinInstAlignment = 4;

  // Directiva de alineación: usar .p2align (GNU-style) en vez de .align
  AlignmentIsInBytes = false;

  // Soporte de información de depuración DWARF
  SupportsDebugInformation = true;

  // Directiva BSS usando .section (ELF estándar)
  UsesELFSectionDirectiveForBSS = true;

  // Prefijo de símbolos locales (etiquetas temporales)
  InternalSymbolPrefix = ".L";
}

void ISA32_LMMCAsmInfo::printSpecifierExpr(raw_ostream &OS,
                                           const MCSpecifierExpr &Expr) const {
  if (Expr.getSpecifier() == 0) {
    printExpr(OS, *Expr.getSubExpr());
    return;
  }

  switch (Expr.getSpecifier()) {
  default:
    llvm_unreachable("Especificador de expresiones no válido");
  case ISA32_LM::S_HI16:
    OS << "%hi";
    break;
  case ISA32_LM::S_LO16:
    OS << "%lo";
    break;
  }

  OS << '(';
  printExpr(OS, *Expr.getSubExpr());
  OS << ')';
}
