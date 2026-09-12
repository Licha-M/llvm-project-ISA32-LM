//===-- ISA32_LMInstrInfo.td - ISA32_LM Instruction Definitions -----------===//
//
// Definición completa de las instrucciones de la ISA32_LM.
//
// Este archivo asume que los siguientes archivos ya han sido incluidos
// (el include lo realiza ISA32_LM.td en el orden correcto):
//   1. ISA32_LMRegisterInfo.td   — registros y RegisterClass
//   2. ISA32_LMInstrFormats.td   — clases base de formato y valores de OpCode
//
// Convenciones de codificación (del assembler.py):
//   build_word(tipo, opcode, operandos) → word de 32 bits
//   [31:29]=Tipo  [28:24]=OpCode  [23:0]=Operandos
//
// Convención de llamada:
//   Volátiles   (caller-saved): R1–R7
//   Preservados (callee-saved): R8–R13
//   Reservados:  R0 (=0), R14 (SP), R15 (scratch asm)
//
// Referencia canónica: assembler.py (Procesador-32-bits)
//===----------------------------------------------------------------------===//


//===----------------------------------------------------------------------===//
// Operandos personalizados
//===----------------------------------------------------------------------===//

// Inmediato sin signo de 16 bits — para LDI
def uimm16 : Operand<i32> {
  let PrintMethod = "printImm16Operand";
}

// Inmediato con signo de 16 bits — para ADI, LOD offset, STR offset
def simm16 : Operand<i32> {
  let PrintMethod = "printSImm16Operand";
}

// Código de condición de BRH (4 bits)
// Codificación (assembler.py → CONDITIONS):
//   0000 = EQ  (Zero)         0001 = NE  (Not Zero)
//   0010 = N   (Negative)     0011 = NN  (Not Negative)
//   0100 = C   (Carry)        0101 = NC  (Not Carry)
//   0110 = OV  (Overflow)     0111 = NOV (Not Overflow)
def brh_cond : Operand<i32> {
  let PrintMethod = "printBRHCondOperand";
}

// Destino de salto (basic block)
def brh_target : Operand<OtherVT> {
  let PrintMethod = "printBrTargetOperand";
}


//===----------------------------------------------------------------------===//
// Nodos SDNode personalizados para patrones DAG
//===----------------------------------------------------------------------===//

// Llamada a subrutina (CAL)
def SDT_ISA32LM_Call : SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;
def isa32lm_call     : SDNode<"ISA32_LMISD::CALL", SDT_ISA32LM_Call,
                               [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue,
                                SDNPVariadic]>;

// Retorno de subrutina (RET)
def isa32lm_ret_glue : SDNode<"ISA32_LMISD::RET_GLUE", SDTNone,
                               [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

// Branch condicional (BRH)
// Perfil: 0 resultados, 4 operandos: LHS, RHS, CC, BB-destino
def SDT_ISA32LM_BrCC : SDTypeProfile<0, 4,
                         [SDTCisSameAs<0, 1>,
                          SDTCisVT<2, i32>,
                          SDTCisVT<3, OtherVT>]>;
def isa32lm_brcc : SDNode<"ISA32_LMISD::BR_CC", SDT_ISA32LM_BrCC,
                           [SDNPHasChain]>;

// Wrapper para direcciones globales (necesario para codegen)
def SDT_ISA32LM_Wrapper : SDTypeProfile<1, 1,
                             [SDTCisSameAs<0, 1>, SDTCisPtrTy<0>]>;
def isa32lm_wrapper : SDNode<"ISA32_LMISD::Wrapper", SDT_ISA32LM_Wrapper>;

// GOF: lee el flag de overflow/carry de la última operación aritmética
def isa32lm_gof : SDNode<"ISA32_LMISD::GOF",
                          SDTypeProfile<1, 0, [SDTCisVT<0, i32>]>,
                          [SDNPHasChain, SDNPSideEffect]>;

// HLT: detiene el núcleo
def isa32lm_hlt : SDNode<"ISA32_LMISD::HLT", SDTNone,
                          [SDNPHasChain, SDNPSideEffect]>;

// SCL: syscall
def isa32lm_scl : SDNode<"ISA32_LMISD::SCL", SDTNone,
                          [SDNPHasChain, SDNPSideEffect]>;

// SRT: retorno al SO (desde EPC)
def isa32lm_srt : SDNode<"ISA32_LMISD::SRT", SDTNone,
                          [SDNPHasChain, SDNPSideEffect]>;

// Select condicional (para implementar operaciones condicionales en codegen)
def SDT_ISA32LM_SelectCC : SDTypeProfile<1, 5,
                              [SDTCisSameAs<1, 2>,
                               SDTCisSameAs<0, 4>,
                               SDTCisSameAs<4, 5>]>;
def isa32lm_selectcc : SDNode<"ISA32_LMISD::SELECT_CC",
                               SDT_ISA32LM_SelectCC>;

// Secuencias de llamada (ajuste del stack en call sequences)
def SDT_ISA32LM_CallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>,
                                               SDTCisVT<1, i32>]>;
def SDT_ISA32LM_CallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i32>,
                                              SDTCisVT<1, i32>]>;
def isa32lm_callseq_start : SDNode<"ISD::CALLSEQ_START",
                                    SDT_ISA32LM_CallSeqStart,
                                    [SDNPHasChain, SDNPOutGlue]>;
def isa32lm_callseq_end   : SDNode<"ISD::CALLSEQ_END",
                                    SDT_ISA32LM_CallSeqEnd,
                                    [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;


//===----------------------------------------------------------------------===//
// PatLeaf helpers para inmediatos
//===----------------------------------------------------------------------===//

// Inmediato que cabe en 15 bits sin signo [0, 32767] (evita signo extendido negativo en LDI)
def uimm16_pat : PatLeaf<(imm), [{
  return isUInt<15>(N->getZExtValue());
}]>;

// Inmediato que cabe en 16 bits con signo [-32768, 32767]
def simm16_pat : PatLeaf<(imm), [{
  return isInt<16>(N->getSExtValue());
}]>;


//===----------------------------------------------------------------------===//
// Instrucciones — Grupo 0: Sin operandos
//===----------------------------------------------------------------------===//

// NOP — No operation
// Encoding: Tipo=000 OpCode=00000 [23:0]=0
let hasSideEffects = 0 in
def NOP : Fmt_None<OPC_NOP, "NOP", []>;

// HLT — Detiene el núcleo hasta recibir interrupción (kernel-only)
// Encoding: Tipo=000 OpCode=00001 [23:0]=0
let hasSideEffects = 1, isTerminator = 1, isBarrier = 1 in
def HLT : Fmt_None<OPC_HLT, "HLT", [(isa32lm_hlt)]>;

// RET — Retorno de subrutina (el hardware mantiene la dirección de retorno)
// Encoding: Tipo=000 OpCode=10001 [23:0]=0
let isReturn = 1, isTerminator = 1, isBarrier = 1,
    isNotDuplicable = 1, hasSideEffects = 1 in
def RET : Fmt_None<OPC_RET, "RET", [(isa32lm_ret_glue)]>;

// SCL — Llamada al sistema operativo (syscall)
// Encoding: Tipo=000 OpCode=10110 [23:0]=0
let hasSideEffects = 1, isCall = 1, Defs = [R1, R2, R3, R4, R5, R6, R7] in
def SCL : Fmt_None<OPC_SCL, "SCL", [(isa32lm_scl)]>;

// SRT — Retorno al SO (a la dirección en EPC); uso exclusivo del kernel
// Encoding: Tipo=000 OpCode=10111 [23:0]=0
let hasSideEffects = 1, isReturn = 1, isTerminator = 1,
    isBarrier = 1, isNotDuplicable = 1 in
def SRT : Fmt_None<OPC_SRT, "SRT", [(isa32lm_srt)]>;


//===----------------------------------------------------------------------===//
// Instrucciones — Grupo 1: Formato R (tres registros Rs1, Rs2, Rd)
//
// Encoding: [23:20]=Rs1 [19:16]=Rs2 [15:12]=Rd [11:0]=0
// Sintaxis: Op Rs1, Rs2, Rd
//
// Variante normal (Tipo=000): modifica flags (Zero, Neg, Carry, Overflow)
// Variante SLT   (Tipo=100): operación silenciosa, NO modifica flags
//===----------------------------------------------------------------------===//

// ─── Variante normal (Tipo=000) ───────────────────────────────────────────

// ADD Rs1, Rs2, Rd — Rs1 + Rs2 → Rd
def ADD_RRR : Fmt_R<OPC_ADD, 0b000,
    (outs GPR:$rd),
    (ins GPR:$rs1, GPR:$rs2),
    "ADD $rs1, $rs2, $rd",
    [(set i32:$rd, (add i32:$rs1, i32:$rs2))]> {
  bits<4> rs1; bits<4> rs2; bits<4> rd;
}

// SUB Rs1, Rs2, Rd — Rs1 - Rs2 → Rd
def SUB_RRR : Fmt_R<OPC_SUB, 0b000,
    (outs GPR:$rd),
    (ins GPR:$rs1, GPR:$rs2),
    "SUB $rs1, $rs2, $rd",
    [(set i32:$rd, (sub i32:$rs1, i32:$rs2))]> {
  bits<4> rs1; bits<4> rs2; bits<4> rd;
}

// ─── CMP: Alias de comparación ─────────────────────────────────────────────
// CMP Rs1, Rs2 es un alias de ensamblador para SUB Rs1, Rs2, R0.
// El hardware calcula (Rs1 - Rs2), actualiza SR9 (Flags) y SR8 (EFlags),
// y descarta el resultado en R0 (wire-zero).
def : InstAlias<"CMP $rs1, $rs2", (SUB_RRR GPR:$rs1, GPR:$rs2, R0)>;

// MUL Rs1, Rs2, Rd — Rs1 × Rs2 → Rd
def MUL_RRR : Fmt_R<OPC_MUL, 0b000,
    (outs GPR:$rd),
    (ins GPR:$rs1, GPR:$rs2),
    "MUL $rs1, $rs2, $rd",
    [(set i32:$rd, (mul i32:$rs1, i32:$rs2))]> {
  bits<4> rs1; bits<4> rs2; bits<4> rd;
}

// DIV Rs1, Rs2, Rd — Rs1 ÷ Rs2 → Rd (división sin signo)
def DIV_RRR : Fmt_R<OPC_DIV, 0b000,
    (outs GPR:$rd),
    (ins GPR:$rs1, GPR:$rs2),
    "DIV $rs1, $rs2, $rd",
    [(set i32:$rd, (udiv i32:$rs1, i32:$rs2))]> {
  bits<4> rs1; bits<4> rs2; bits<4> rd;
}

// NOR Rs1, Rs2, Rd — NOT(Rs1 OR Rs2) → Rd
def NOR_RRR : Fmt_R<OPC_NOR, 0b000,
    (outs GPR:$rd),
    (ins GPR:$rs1, GPR:$rs2),
    "NOR $rs1, $rs2, $rd",
    [(set i32:$rd, (not (or i32:$rs1, i32:$rs2)))]> {
  bits<4> rs1; bits<4> rs2; bits<4> rd;
}

// AND Rs1, Rs2, Rd — Rs1 AND Rs2 → Rd
def AND_RRR : Fmt_R<OPC_AND, 0b000,
    (outs GPR:$rd),
    (ins GPR:$rs1, GPR:$rs2),
    "AND $rs1, $rs2, $rd",
    [(set i32:$rd, (and i32:$rs1, i32:$rs2))]> {
  bits<4> rs1; bits<4> rs2; bits<4> rd;
}

// XOR Rs1, Rs2, Rd — Rs1 XOR Rs2 → Rd
def XOR_RRR : Fmt_R<OPC_XOR, 0b000,
    (outs GPR:$rd),
    (ins GPR:$rs1, GPR:$rs2),
    "XOR $rs1, $rs2, $rd",
    [(set i32:$rd, (xor i32:$rs1, i32:$rs2))]> {
  bits<4> rs1; bits<4> rs2; bits<4> rd;
}

// RSH Rs1, Rs2, Rd — Rs1 >> Rs2 → Rd (shift lógico a derecha)
def RSH_RRR : Fmt_R<OPC_RSH, 0b000,
    (outs GPR:$rd),
    (ins GPR:$rs1, GPR:$rs2),
    "RSH $rs1, $rs2, $rd",
    [(set i32:$rd, (srl i32:$rs1, i32:$rs2))]> {
  bits<4> rs1; bits<4> rs2; bits<4> rd;
}

// LSH Rs1, Rs2, Rd — Rs1 << Rs2 → Rd (shift lógico a izquierda)
def LSH_RRR : Fmt_R<OPC_LSH, 0b000,
    (outs GPR:$rd),
    (ins GPR:$rs1, GPR:$rs2),
    "LSH $rs1, $rs2, $rd",
    [(set i32:$rd, (shl i32:$rs1, i32:$rs2))]> {
  bits<4> rs1; bits<4> rs2; bits<4> rd;
}

// ─── Variante SLT (Tipo=100): silenciosa, no modifica flags ───────────────
// Fuente: assembler.py — tipo==0b100 y mnemónico en {ADD,SUB,...}

let hasSideEffects = 0 in {

def SLT_ADD_RRR : Fmt_R<OPC_ADD, 0b100,
    (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
    "SLT ADD $rs1, $rs2, $rd", []> {
  bits<4> rs1; bits<4> rs2; bits<4> rd;
}

def SLT_SUB_RRR : Fmt_R<OPC_SUB, 0b100,
    (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
    "SLT SUB $rs1, $rs2, $rd", []> {
  bits<4> rs1; bits<4> rs2; bits<4> rd;
}

def SLT_MUL_RRR : Fmt_R<OPC_MUL, 0b100,
    (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
    "SLT MUL $rs1, $rs2, $rd", []> {
  bits<4> rs1; bits<4> rs2; bits<4> rd;
}

def SLT_DIV_RRR : Fmt_R<OPC_DIV, 0b100,
    (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
    "SLT DIV $rs1, $rs2, $rd", []> {
  bits<4> rs1; bits<4> rs2; bits<4> rd;
}

def SLT_NOR_RRR : Fmt_R<OPC_NOR, 0b100,
    (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
    "SLT NOR $rs1, $rs2, $rd", []> {
  bits<4> rs1; bits<4> rs2; bits<4> rd;
}

def SLT_AND_RRR : Fmt_R<OPC_AND, 0b100,
    (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
    "SLT AND $rs1, $rs2, $rd", []> {
  bits<4> rs1; bits<4> rs2; bits<4> rd;
}

def SLT_XOR_RRR : Fmt_R<OPC_XOR, 0b100,
    (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
    "SLT XOR $rs1, $rs2, $rd", []> {
  bits<4> rs1; bits<4> rs2; bits<4> rd;
}

def SLT_RSH_RRR : Fmt_R<OPC_RSH, 0b100,
    (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
    "SLT RSH $rs1, $rs2, $rd", []> {
  bits<4> rs1; bits<4> rs2; bits<4> rd;
}

def SLT_LSH_RRR : Fmt_R<OPC_LSH, 0b100,
    (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
    "SLT LSH $rs1, $rs2, $rd", []> {
  bits<4> rs1; bits<4> rs2; bits<4> rd;
}

} // end let hasSideEffects = 0 (SLT variants)


//===----------------------------------------------------------------------===//
// Instrucciones — Grupo 2: GOF (Get Overflow/Carry Flag)
//
// Encoding: Tipo=000 OpCode=01011 [23:20]=Rd [19:0]=0
// Sintaxis: GOF Rd
//
// Captura el flag de overflow/carry de la última operación aritmética.
// No tiene patrón DAG directo (depende del estado de flags del hardware).
//===----------------------------------------------------------------------===//

let hasSideEffects = 1 in
def GOF : Fmt_RA<OPC_GOF,
    (outs GPR:$ra),
    (ins),
    "GOF $ra",
    [(set i32:$ra, (isa32lm_gof))]> {
  bits<4> ra;
}


//===----------------------------------------------------------------------===//
// Instrucciones — Grupo 3: LDI (Load Immediate)
//
// Encoding normal (Tipo=000): [23:20]=Rd [19:16]=0 [15:0]=IMM16
// Encoding alto   (Tipo=100): igual pero carga en bits [31:16] del registro
// Sintaxis: LDI Rd, Inmediato
//===----------------------------------------------------------------------===//

// LDI Rd, IMM16 — carga IMM16 (sin signo) en los 16 bits bajos de Rd
let isReMaterializable = 1 in
def LDI : Fmt_I<OPC_LDI, 0b000,
    (outs GPR:$rd),
    (ins uimm16:$imm16),
    "LDI $rd, $imm16",
    [(set i32:$rd, uimm16_pat:$imm16)]> {
  bits<4>  rd;
  bits<16> imm16;
}

// H LDI Rd, IMM16 — carga IMM16 en los 16 bits altos de Rd (Tipo=100)
// Usado por la expansión de LDI de 32 bits (pseudoinstrucción LDI32).
let isReMaterializable = 1, hasSideEffects = 0 in
def HLDI : Fmt_I<OPC_LDI, 0b100,
    (outs GPR:$rd),
    (ins uimm16:$imm16),
    "H LDI $rd, $imm16",
    []> {
  bits<4>  rd;
  bits<16> imm16;
}


//===----------------------------------------------------------------------===//
// Instrucciones — Grupo 4: ADI (Add Immediate Signed)
//
// Encoding: [23:20]=Rd [19:16]=0 [15:0]=SIMM16
// Sintaxis: ADI Rd, Inmediato
//
// Modo SLT (Tipo=100): operación silenciosa, no modifica flags.
// Se usa en la expansión de LDI de 32 bits para la parte baja.
//===----------------------------------------------------------------------===//

// ADI Rd, SIMM16 — Rd + SIMM16 → Rd (actualiza flags)
def ADI : Fmt_I<OPC_ADI, 0b000,
    (outs GPR:$rd_out),
    (ins GPR:$rd, simm16:$imm16),
    "ADI $rd, $imm16",
    [(set i32:$rd_out, (add i32:$rd, simm16_pat:$imm16))]> {
  bits<4>  rd;
  bits<16> imm16;
  let Constraints = "$rd_out = $rd";
}

// SLT ADI Rd, SIMM16 — Rd + SIMM16 → Rd (silencioso, no modifica flags)
let hasSideEffects = 0 in
def ADI_SLT : Fmt_I<OPC_ADI, 0b100,
    (outs GPR:$rd_out),
    (ins GPR:$rd, simm16:$imm16),
    "SLT ADI $rd, $imm16",
    []> {
  bits<4>  rd;
  bits<16> imm16;
  let Constraints = "$rd_out = $rd";
}


//===----------------------------------------------------------------------===//
// Instrucciones — Grupo 5: JMP (Jump Unconditional Indirect)
//
// Encoding: Tipo=000 OpCode=01110 [23:20]=0 [19:16]=Rs1 [15:0]=0
// Sintaxis: JMP Rs1
//===----------------------------------------------------------------------===//

let isBranch = 1, isTerminator = 1, isBarrier = 1,
    isIndirectBranch = 1 in
def JMP : Fmt_RB<OPC_JMP,
    (outs),
    (ins GPR:$rb),
    "JMP $rb",
    [(brind i32:$rb)]> {
  bits<4> rb;
}


//===----------------------------------------------------------------------===//
// Instrucciones — Grupo 6: BRH (Branch Conditional)
//
// Encoding: Tipo=000 OpCode=01111 [23:20]=Cond(4b) [19:16]=Rs1 [15:0]=0
// Sintaxis: BRH Cond, Rs1
//
// Condiciones (4 bits, assembler.py → CONDITIONS):
//   0000=EQ  0001=NE  0010=N  0011=NN
//   0100=C   0101=NC  0110=OV 0111=NOV
//===----------------------------------------------------------------------===//

let isBranch = 1, isTerminator = 1, hasDelaySlot = 0 in
def BRH : Fmt_BRH<
    (outs),
    (ins brh_cond:$cond, GPR:$rs1),
    "BRH $cond, $rs1",
    []> {
  bits<4> cond;
  bits<4> rs1;
}


//===----------------------------------------------------------------------===//
// Instrucciones — Grupo 7: CAL (Call Subroutine)
//
// Encoding: Tipo=000 OpCode=10000 [23:20]=0 [19:16]=Rs1 [15:0]=0
// Sintaxis: CAL Rs1
//
// El hardware guarda la dirección de retorno internamente (registro de link).
//===----------------------------------------------------------------------===//

let isCall = 1, hasSideEffects = 1, Defs = [R1, R2, R3, R4, R5, R6, R7] in
def CAL : Fmt_RB<OPC_CAL,
    (outs),
    (ins GPR:$rb),
    "CAL $rb",
    [(isa32lm_call i32:$rb)]> {
  bits<4> rb;
}


//===----------------------------------------------------------------------===//
// Instrucciones — Grupo 8: LOD (Load from Memory)
//
// Encoding: [31:29]=Tipo_mem [28:24]=10010 [23:20]=Rs1_Base [19:16]=Rd [15:0]=Offset16
// Sintaxis: <tipo> LOD Rs1_Base, Rd, Offset
//
// Tipos de acceso a memoria (campo Tipo):
//   000 = CHAR  (8 bits,  zero-extend)
//   001 = SHORT (16 bits, zero-extend)
//   010 = INT   (32 bits)
//===----------------------------------------------------------------------===//

// LOD.CHAR — carga 8 bits (byte) desde Mem[Rs1_Base + Offset] → Rd
let mayLoad = 1, hasSideEffects = 0 in
def LOD_CHAR : Fmt_Mem<OPC_LOD, 0b000,
    (outs GPR:$rs2_dest),
    (ins GPR:$rs1_base, simm16:$offset),
    "CHAR LOD $rs1_base, $rs2_dest, $offset",
    [(set i32:$rs2_dest, (zextloadi8 (add i32:$rs1_base, simm16_pat:$offset)))]> {
  bits<4>  rs1_base;
  bits<4>  rs2_dest;
  bits<16> offset;
}

// LOD.SHORT — carga 16 bits desde Mem[Rs1_Base + Offset] → Rd
let mayLoad = 1, hasSideEffects = 0 in
def LOD_SHORT : Fmt_Mem<OPC_LOD, 0b001,
    (outs GPR:$rs2_dest),
    (ins GPR:$rs1_base, simm16:$offset),
    "SHORT LOD $rs1_base, $rs2_dest, $offset",
    [(set i32:$rs2_dest, (zextloadi16 (add i32:$rs1_base, simm16_pat:$offset)))]> {
  bits<4>  rs1_base;
  bits<4>  rs2_dest;
  bits<16> offset;
}

// LOD.INT — carga 32 bits desde Mem[Rs1_Base + Offset] → Rd
let mayLoad = 1, hasSideEffects = 0 in
def LOD_INT : Fmt_Mem<OPC_LOD, 0b010,
    (outs GPR:$rs2_dest),
    (ins GPR:$rs1_base, simm16:$offset),
    "INT LOD $rs1_base, $rs2_dest, $offset",
    [(set i32:$rs2_dest, (load (add i32:$rs1_base, simm16_pat:$offset)))]> {
  bits<4>  rs1_base;
  bits<4>  rs2_dest;
  bits<16> offset;
}


//===----------------------------------------------------------------------===//
// Instrucciones — Grupo 9: STR (Store to Memory)
//
// Encoding: [31:29]=Tipo_mem [28:24]=10011 [23:20]=Rs1_Base [19:16]=Rs_Src [15:0]=Offset16
// Sintaxis: <tipo> STR Rs1_Base, Rs_Src, Offset
//===----------------------------------------------------------------------===//

// STR.CHAR — almacena 8 bits: Rs_Src → Mem[Rs1_Base + Offset]
let mayStore = 1, hasSideEffects = 0 in
def STR_CHAR : Fmt_Mem<OPC_STR, 0b000,
    (outs),
    (ins GPR:$rs1_base, GPR:$rs2_dest, simm16:$offset),
    "CHAR STR $rs1_base, $rs2_dest, $offset",
    [(truncstorei8 i32:$rs2_dest, (add i32:$rs1_base, simm16_pat:$offset))]> {
  bits<4>  rs1_base;
  bits<4>  rs2_dest;
  bits<16> offset;
}

// STR.SHORT — almacena 16 bits: Rs_Src → Mem[Rs1_Base + Offset]
let mayStore = 1, hasSideEffects = 0 in
def STR_SHORT : Fmt_Mem<OPC_STR, 0b001,
    (outs),
    (ins GPR:$rs1_base, GPR:$rs2_dest, simm16:$offset),
    "SHORT STR $rs1_base, $rs2_dest, $offset",
    [(truncstorei16 i32:$rs2_dest, (add i32:$rs1_base, simm16_pat:$offset))]> {
  bits<4>  rs1_base;
  bits<4>  rs2_dest;
  bits<16> offset;
}

// STR.INT — almacena 32 bits: Rs_Src → Mem[Rs1_Base + Offset]
let mayStore = 1, hasSideEffects = 0 in
def STR_INT : Fmt_Mem<OPC_STR, 0b010,
    (outs),
    (ins GPR:$rs1_base, GPR:$rs2_dest, simm16:$offset),
    "INT STR $rs1_base, $rs2_dest, $offset",
    [(store i32:$rs2_dest, (add i32:$rs1_base, simm16_pat:$offset))]> {
  bits<4>  rs1_base;
  bits<4>  rs2_dest;
  bits<16> offset;
}


//===----------------------------------------------------------------------===//
// Instrucciones — Grupo 10: CYE (Copy Special Register → General Register)
//
// Encoding: Tipo=000 OpCode=10100 [23:20]=RC_esp [19:16]=Rd_gp [15:0]=0
// Sintaxis: CYE RC_esp, Rd_gp
//===----------------------------------------------------------------------===//

let hasSideEffects = 1 in
def CYE : Fmt_CYE<
    (outs GPR:$ra_gp),
    (ins SREG:$rc_sp),
    "CYE $rc_sp, $ra_gp",
    []> {
  bits<4> rc_sp;
  bits<4> ra_gp;
}


//===----------------------------------------------------------------------===//
// Instrucciones — Grupo 11: CYR (Copy General Register → Special Register)
//
// Encoding: Tipo=000 OpCode=10101 [23:20]=Rs_gp [19:16]=RC_esp [15:0]=0
// Sintaxis: CYR Rs_gp, RC_esp
// Uso exclusivo del kernel.
//===----------------------------------------------------------------------===//

let hasSideEffects = 1 in
def CYR : Fmt_CYR<
    (outs SREG:$rc_sp),
    (ins GPR:$ra_gp),
    "CYR $ra_gp, $rc_sp",
    []> {
  bits<4> ra_gp;
  bits<4> rc_sp;
}


//===----------------------------------------------------------------------===//
// Pseudoinstrucciones
//===----------------------------------------------------------------------===//

// ─── LDI de 32 bits ───────────────────────────────────────────────────────
//
// Carga un valor de 32 bits en un registro en 3 instrucciones:
//   1) LDI     Rd, 0              — poner el registro a 0
//   2) H LDI   Rd, high16_comp   — parte alta (compensada por signo de ADI)
//   3) SLT ADI Rd, low16         — parte baja (silenciosa, sign-extend)
//
// Fuente: expand_ldi_32() en assembler.py
// Generada por el lowering cuando el inmediato no cabe en 16 bits.

let isReMaterializable = 1, isAsCheapAsAMove = 1 in
def LDI32 : Pseudo<
    (outs GPR:$rd),
    (ins i32imm:$imm32),
    "# LDI32 $rd, $imm32",
    [(set i32:$rd, imm:$imm32)]>;

// ─── Salto incondicional a dirección de 32 bits ───────────────────────────
//
// Expansión de JMP a etiqueta (3 instrucciones):
//   1) H LDI   R15, high16_comp
//   2) SLT ADI R15, low16
//   3) JMP R15
//
// R15 se usa como scratch register. Fuente: expand_label_address() en assembler.py.

let isTerminator = 1, isBranch = 1, isBarrier = 1 in
def JMP32_PSEUDO : Pseudo<
    (outs),
    (ins brh_target:$target),
    "# JMP32 $target",
    [(br bb:$target)]>;

// ─── Llamada a subrutina con dirección de 32 bits ─────────────────────────
//
// Expansión de CAL a etiqueta (mismo patrón que JMP32):
//   1) H LDI   R15, high16_comp
//   2) SLT ADI R15, low16
//   3) CAL R15

let isCall = 1, hasSideEffects = 1, Defs = [R1, R2, R3, R4, R5, R6, R7] in
def CAL32_PSEUDO : Pseudo<
    (outs),
    (ins i32imm:$target),
    "# CAL32 $target",
    [(isa32lm_call tglobaladdr:$target)]>;

// ─── Branch condicional con dirección de 32 bits ──────────────────────────
//
// Expansión de BRH a etiqueta (3 instrucciones):
//   1) H LDI   R15, high16_comp
//   2) SLT ADI R15, low16
//   3) BRH COND, R15

let isTerminator = 1, isBranch = 1 in
def BRH32_PSEUDO : Pseudo<
    (outs),
    (ins GPR:$lhs, GPR:$rhs, i32imm:$cc, brh_target:$dst),
    "# BRH32 $lhs, $rhs, $cc, $dst",
    [(isa32lm_brcc i32:$lhs, i32:$rhs, i32:$cc, bb:$dst)]>;

// ─── Ajuste de stack frame (call sequence) ────────────────────────────────
let isCodeGenOnly = 1 in {
  def ADJCALLSTACKDOWN : Pseudo<
      (outs),
      (ins i32imm:$amt1, i32imm:$amt2),
      "#ADJCALLSTACKDOWN $amt1 $amt2",
      [(isa32lm_callseq_start timm:$amt1, timm:$amt2)]>;
  def ADJCALLSTACKUP : Pseudo<
      (outs),
      (ins i32imm:$amt1, i32imm:$amt2),
      "#ADJCALLSTACKUP $amt1 $amt2",
      [(isa32lm_callseq_end timm:$amt1, timm:$amt2)]>;
}

// ─── Select condicional (implementación de ternario en codegen) ───────────
let usesCustomInserter = 1, isCodeGenOnly = 1 in {
  def SELECT_CC : Pseudo<
      (outs GPR:$dst),
      (ins GPR:$lhs, GPR:$rhs, i32imm:$cc, GPR:$tval, GPR:$fval),
      "# SELECT_CC $dst = $lhs $cc $rhs ? $tval : $fval",
      [(set i32:$dst,
        (isa32lm_selectcc i32:$lhs, i32:$rhs,
                          (i32 timm:$cc),
                          i32:$tval, i32:$fval))]>;
}

//===----------------------------------------------------------------------===//
// Patrones DAG adicionales
//===----------------------------------------------------------------------===//

// ─── Cargas inmediatas ────────────────────────────────────────────────────

// Un valor de 32 bits que no cabe en 16 bits → LDI32 (pseudo).
def : Pat<(i32 imm:$imm), (LDI32 imm:$imm)>;

// Valores de 16 bits sin signo → LDI directamente.
def : Pat<(i32 uimm16_pat:$imm), (LDI (i32 uimm16_pat:$imm))>;

// ─── Accesos a memoria con offset ────────────────────────────────────────

def : Pat<(i32 (extloadi8 (add i32:$rs1_base, simm16_pat:$offset))),
          (LOD_CHAR GPR:$rs1_base, $offset)>;

def : Pat<(i32 (extloadi16 (add i32:$rs1_base, simm16_pat:$offset))),
          (LOD_SHORT GPR:$rs1_base, $offset)>;

// ─── Accesos a memoria sin offset (offset = 0) ───────────────────────────

// Carga byte desde puntero directo
def : Pat<(i32 (zextloadi8 i32:$ptr)),
          (LOD_CHAR GPR:$ptr, 0)>;
def : Pat<(i32 (extloadi8 i32:$ptr)),
          (LOD_CHAR GPR:$ptr, 0)>;

// Carga halfword desde puntero directo
def : Pat<(i32 (zextloadi16 i32:$ptr)),
          (LOD_SHORT GPR:$ptr, 0)>;
def : Pat<(i32 (extloadi16 i32:$ptr)),
          (LOD_SHORT GPR:$ptr, 0)>;

// Carga word desde puntero directo
def : Pat<(i32 (load i32:$ptr)),
          (LOD_INT GPR:$ptr, 0)>;

// Almacena byte
def : Pat<(truncstorei8 i32:$val, i32:$ptr),
          (STR_CHAR GPR:$ptr, GPR:$val, 0)>;

// Almacena halfword
def : Pat<(truncstorei16 i32:$val, i32:$ptr),
          (STR_SHORT GPR:$ptr, GPR:$val, 0)>;

// Almacena word
def : Pat<(store i32:$val, i32:$ptr),
          (STR_INT GPR:$ptr, GPR:$val, 0)>;

// ─── Llamadas ─────────────────────────────────────────────────────────────
def : Pat<(isa32lm_call tglobaladdr:$dst),   (CAL32_PSEUDO tglobaladdr:$dst)>;
def : Pat<(isa32lm_call texternalsym:$dst),  (CAL32_PSEUDO texternalsym:$dst)>;
def : Pat<(isa32lm_call GPR:$dst),           (CAL GPR:$dst)>;

// ─── ADI con simm16 ──────────────────────────────────────────────────────
def : Pat<(add i32:$ra, simm16_pat:$imm),
          (ADI GPR:$ra, simm16:$imm)>;

// ─── Wrapper para GlobalValue (direcciones globales) ──────────────────────
def : Pat<(isa32lm_wrapper tglobaladdr:$in),   (LDI32 tglobaladdr:$in)>;
def : Pat<(isa32lm_wrapper tconstpool:$in),    (LDI32 tconstpool:$in)>;
def : Pat<(isa32lm_wrapper tjumptable:$in),    (LDI32 tjumptable:$in)>;
def : Pat<(isa32lm_wrapper tblockaddress:$in), (LDI32 tblockaddress:$in)>;

// ─── Operaciones lógicas sintéticas ───────────────────────────────────────
// De Morgan: a | b = NOT(NOR(a, b)) = NOR(NOR(a, b), NOR(a, b))
def : Pat<(or GPR:$rs1, GPR:$rs2),
          (NOR_RRR (NOR_RRR GPR:$rs1, GPR:$rs2),
                   (NOR_RRR GPR:$rs1, GPR:$rs2))>;

// // ─── Materialización de FrameIndex ────────────────────────────────────────
// def : Pat<(i32 tframeindex:$fi),
//           (ADI (ADD_RRR R14, R0), tframeindex:$fi)>;

defm : RemapAllTargetPseudoPointerOperands<GPR>;
