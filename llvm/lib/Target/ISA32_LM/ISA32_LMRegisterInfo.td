//===-- ISA32_LMRegisterInfo.td - ISA32_LM Register Definitions -----------===//
//
// Definición del banco de registros para la arquitectura ISA32_LM.
//
// Arquitectura: RISC custom 32 bits, Little-Endian.
//
// Banco de registros general (GPR):
//   R0–R15 (32 bits cada uno, sin sub-registros).
//   R0  = constante 0 (wire-zero). Siempre reservado.
//   R14 = Stack Pointer (SP) autónomo por hardware. Siempre reservado.
//   R15 = scratch para el ensamblador / cálculos de salto. Siempre reservado.
//
// Banco de registros especial (SREG):
//   SR0–SR15 (32 bits, solo accesibles por CYE/CYR).
//   SR0 = constante 0. SR13–SR15 uso exclusivo del kernel.
//
// Convención de llamada:
//   Volátiles:   R1–R7   (el llamador guarda)
//   Preservados: R8–R13  (el llamado guarda)
//   Especiales:  R0 (zero), R14 (SP), R15 (scratch asm)
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Banco de registros general: R0–R15 (32 bits cada uno)
//
// Los registros se codifican con 4 bits (0–15) en los campos RA, RB, RC.
// No existen sub-registros; el ancho de acceso es siempre 32 bits.
//===----------------------------------------------------------------------===//


let Namespace = "ISA32_LM" in {

  //--------------------------------------------------------------------------
  // Clase base para registros de propósito general (GPR)
  //--------------------------------------------------------------------------
  class GPReg<bits<16> Enc, string n, list<string> altNames = []>
      : Register<n, altNames> {
    let HWEncoding = Enc;
    let Namespace  = "ISA32_LM";
  }

  //--------------------------------------------------------------------------
  // Registros R0–R15
  //   R0  → constante 0 (wire-zero); LLVM debe reservarlo.
  //   R14 → Stack Pointer autónomo por hardware; LLVM debe reservarlo.
  //   R15 → scratch exclusivo del ensamblador; LLVM nunca lo asigna.
  //--------------------------------------------------------------------------
  def R0  : GPReg< 0, "R0">,  DwarfRegNum<[0]>;
  def R1  : GPReg< 1, "R1">,  DwarfRegNum<[1]>;
  def R2  : GPReg< 2, "R2">,  DwarfRegNum<[2]>;
  def R3  : GPReg< 3, "R3">,  DwarfRegNum<[3]>;
  def R4  : GPReg< 4, "R4">,  DwarfRegNum<[4]>;
  def R5  : GPReg< 5, "R5">,  DwarfRegNum<[5]>;
  def R6  : GPReg< 6, "R6">,  DwarfRegNum<[6]>;
  def R7  : GPReg< 7, "R7">,  DwarfRegNum<[7]>;
  def R8  : GPReg< 8, "R8">,  DwarfRegNum<[8]>;
  def R9  : GPReg< 9, "R9">,  DwarfRegNum<[9]>;
  def R10 : GPReg<10, "R10">, DwarfRegNum<[10]>;
  def R11 : GPReg<11, "R11">, DwarfRegNum<[11]>;
  def R12 : GPReg<12, "R12">, DwarfRegNum<[12]>;
  def R13 : GPReg<13, "R13">, DwarfRegNum<[13]>;
  def R14 : GPReg<14, "R14">, DwarfRegNum<[14]>; // Stack Pointer (SP)
  def R15 : GPReg<15, "R15">, DwarfRegNum<[15]>; // Scratch asm (reservado)

  //--------------------------------------------------------------------------
  // Clase base para registros especiales (SREG)
  //   Solo accesibles mediante CYE (especial→general) / CYR (general→especial).
  //   DWARF numbers: SR0=32 … SR15=47 (offset +32 del banco general).
  //--------------------------------------------------------------------------
  class SPReg<bits<16> Enc, string n, list<string> altNames = []>
      : Register<n, altNames> {
    let HWEncoding = Enc;
    let Namespace  = "ISA32_LM";
  }

  //--------------------------------------------------------------------------
  // Registros SR0–SR15
  //   SR0        = constante 0.
  //   SR8        = EFlags (flags extendidos del procesador).
  //   SR9        = Flags  (flags de condición: Zero, Negative, Carry, Overflow).
  //   SR10       = Carry  (flag de acarreo/prestamo explícito).
  //   SR13–SR15  = uso exclusivo del kernel (guardado de contexto).
  //--------------------------------------------------------------------------
  def SR0  : SPReg< 0, "SR0">,  DwarfRegNum<[32]>;
  def SR1  : SPReg< 1, "SR1">,  DwarfRegNum<[33]>;
  def SR2  : SPReg< 2, "SR2">,  DwarfRegNum<[34]>;
  def SR3  : SPReg< 3, "SR3">,  DwarfRegNum<[35]>;
  def SR4  : SPReg< 4, "SR4">,  DwarfRegNum<[36]>;
  def SR5  : SPReg< 5, "SR5">,  DwarfRegNum<[37]>;
  def SR6  : SPReg< 6, "SR6">,  DwarfRegNum<[38]>;
  def SR7  : SPReg< 7, "SR7">,  DwarfRegNum<[39]>;
  def SR8  : SPReg< 8, "SR8",  ["EFlags", "eflags"]>, DwarfRegNum<[40]>;
  def SR9  : SPReg< 9, "SR9",  ["Flags", "flags"]>,   DwarfRegNum<[41]>;
  def SR10 : SPReg<10, "SR10", ["Carry", "carry"]>,   DwarfRegNum<[42]>;
  def SR11 : SPReg<11, "SR11">, DwarfRegNum<[43]>;
  def SR12 : SPReg<12, "SR12">, DwarfRegNum<[44]>;
  def SR13 : SPReg<13, "SR13">, DwarfRegNum<[45]>; // Kernel-only
  def SR14 : SPReg<14, "SR14">, DwarfRegNum<[46]>; // Kernel-only
  def SR15 : SPReg<15, "SR15">, DwarfRegNum<[47]>; // Kernel-only

} // end Namespace ISA32_LM


//===----------------------------------------------------------------------===//
// Clases de registro (RegisterClass)
//===----------------------------------------------------------------------===//

// GPR32: banco de registros de propósito general de 32 bits.
//
// Política de asignación del allocator:
//   1) Volátiles primero (R1–R7): el llamador guarda, baratos de usar.
//   2) Preservados (R8–R13): el llamado guarda, spill más costoso.
//   3) R14 (SP): reservado, nunca asignado por el allocator.
//   4) R15: reservado como scratch del ensamblador, nunca asignado.
//   5) R0 al final: constante 0, reservado, útil como fuente en patrones.
//
// Nota: R0, R14 y R15 están en la clase para que el backend pueda
// referirse a ellos en patrones. getReservedRegs() los marca como
// reservados, por lo que el allocator nunca los elige.
def GPR32 : RegisterClass<"ISA32_LM", [i32], 32, (add
  // Registros volátiles (caller-saved): R1–R7
  R1, R2, R3, R4, R5, R6, R7,
  // Registros preservados (callee-saved): R8–R13
  R8, R9, R10, R11, R12, R13,
  // Stack Pointer (SP - reservado por C++ getReservedRegs)
  R14,
  // Constante 0 (wire-zero - reservado por C++ getReservedRegs)
  R0
)>;

// Alias canónico: GPR es equivalente a GPR32 en todo el backend.
def GPR : RegisterClass<"ISA32_LM", [i32], 32, (add
  R1, R2, R3, R4, R5, R6, R7,
  R8, R9, R10, R11, R12, R13,
  R14, R0
)>;

// SREG: banco de registros especial de 32 bits.
//   El allocator nunca debe asignar registros especiales a variables de usuario.
//   SR0 es constante 0. SR13–SR15 son de uso exclusivo del kernel.
def SREG : RegisterClass<"ISA32_LM", [i32], 32, (add
  SR1,  SR2,  SR3,  SR4,
  SR5,  SR6,  SR7,  SR8,
  SR9,  SR10, SR11, SR12,
  SR13, SR14, SR15,  // Uso exclusivo del kernel
  SR0                // Constante 0
)>;
