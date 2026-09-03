//===-- ISA32_LM.td - Describe the ISA32_LM Target Machine ----------------===//
//
// Definición principal del target ISA32_LM para LLVM TableGen.
//
// Arquitectura: RISC custom de 32 bits, Little-Endian.
//   · 16 registros de propósito general (R0–R15, 32 bits, sin sub-registros).
//   · 16 registros especiales (SR0–SR15, solo accesibles con CYE/CYR).
//   · Instrucciones de 32 bits fijos, alineadas a 4 bytes.
//   · Un único modo de direccionamiento: base + offset con signo de 16 bits.
//
// Este archivo conecta:
//   ISA32_LMRegisterInfo.td  → definición de registros y RegisterClass
//   ISA32_LMInstrFormats.td  → clases base de formato de instrucción
//   ISA32_LMInstrInfo.td     → definiciones completas de instrucciones
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// Sub-archivos del target
//===----------------------------------------------------------------------===//

include "ISA32_LMRegisterInfo.td"
include "ISA32_LMCallingConv.td"
include "ISA32_LMInstrFormats.td"
include "ISA32_LMInstrInfo.td"

//===----------------------------------------------------------------------===//
// Descriptor de instrucciones
//===----------------------------------------------------------------------===//

def ISA32_LMInstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// Subtarget y procesadores
//===----------------------------------------------------------------------===//

// La ISA32_LM tiene una sola variante por ahora; se puede ampliar después.
def : Processor<"generic", NoItineraries, []>;

//===----------------------------------------------------------------------===//
// Escritor de ensamblador (AsmWriter / InstPrinter)
//===----------------------------------------------------------------------===//

def ISA32_LMInstPrinter : AsmWriter {
  string AsmWriterClassName = "InstPrinter";
  bit isMCAsmWriter = 1;
}

//===----------------------------------------------------------------------===//
// Parser de ensamblador (AsmParser)
//
// Reglas del tokenizador:
//   · Los comentarios empiezan con ';' (comentario hasta fin de línea).
//   · Los registros se prefijan con 'R' (ej. R0, R14).
//   · Las instrucciones son insensibles a mayúsculas/minúsculas.
//===----------------------------------------------------------------------===//

def ISA32_LMAsmParser : AsmParser {
  bit HasMnemonicFirst = 1;
}

def ISA32_LMAsmParserVariant : AsmParserVariant {
  int  Variant               = 0;
  string Name                = "ISA32_LM";
  // ';' introduce comentarios de línea en el ensamblador ISA32_LM
  string CommentDelimiter    = ";";
  // Caracteres que se separan como tokens independientes
  string TokenizingCharacters = "#()[]=:.<>!+*%/";
}

//===----------------------------------------------------------------------===//
// Definición final del target
//===----------------------------------------------------------------------===//

def ISA32_LM : Target {
  let InstructionSet          = ISA32_LMInstrInfo;
  let AssemblyWriters         = [ISA32_LMInstPrinter];
  let AssemblyParsers         = [ISA32_LMAsmParser];
  let AssemblyParserVariants  = [ISA32_LMAsmParserVariant];
}
