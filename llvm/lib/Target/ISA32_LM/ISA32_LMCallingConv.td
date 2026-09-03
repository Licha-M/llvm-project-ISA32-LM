//===-- ISA32_LMCallingConv.td - Calling Conventions ------*- tablegen -*-===//
//
// Reglas de la convención de llamadas (Calling Convention) para ISA32_LM.
//
// Arquitectura: RISC custom de 32 bits, Little-Endian.
//
// Pasaje de Parámetros:
//   · Primeros 7 argumentos enteros (32-bit): R1, R2, R3, R4, R5, R6, R7.
//   · Argumentos de 64 bits (i64): Se dividen en pares (ej. R1+R2, R3+R4, etc.).
//   · Argumentos adicionales: Se pasan en la pila (Stack Grows UP, alignment=4).
//
// Valores de Retorno:
//   · Retorno de 32 bits: R1
//   · Retorno de 64 bits: R1 (bits bajos) y R2 (bits altos)
//
// Registros Preservados (Callee-Saved Registers):
//   · R8, R9, R10, R11, R12, R13
//
// Registros Reservados:
//   · R0  = Wire-Zero (constante 0)
//   · R14 = Stack Pointer (SP)
//   · R15 = Scratch del ensamblador
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Convención de Llamadas Estándar para Argumentos (C Calling Convention)
//===----------------------------------------------------------------------===//

def CC_ISA32_LM : CallingConv<[
  // Promover tipos enteros menores a i32 (i8, i16 -> i32 con extensión de signo o cero)
  CCIfType<[i8, i16], CCPromoteToType<i32>>,

  // Pasar los primeros 7 argumentos de 32 bits en R1 - R7
  CCIfType<[i32], CCAssignToReg<[R1, R2, R3, R4, R5, R6, R7]>>,

  // Cualquier argumento adicional de 32 bits se pasa por la pila.
  // La pila en ISA32_LM crece hacia ARRIBA (Low to High), alineada a 4 bytes.
  CCIfType<[i32], CCAssignToStack<4, 4>>
]>;

//===----------------------------------------------------------------------===//
// Convención de Llamadas para Valores de Retorno (Return Value CC)
//===----------------------------------------------------------------------===//

def RetCC_ISA32_LM : CallingConv<[
  // Promover tipos enteros menores a i32
  CCIfType<[i8, i16], CCPromoteToType<i32>>,

  // Retorno de 32 bits en R1
  CCIfType<[i32], CCAssignToReg<[R1, R2]>>
]>;

//===----------------------------------------------------------------------===//
// Registros Preservados por el Llamado (Callee-Saved Registers - CSR)
//===----------------------------------------------------------------------===//

// Registros que la función llamada debe preservar intactos: R8 a R13.
def CSR_ISA32_LM : CalleeSavedRegs<(add R8, R9, R10, R11, R12, R13)>;
