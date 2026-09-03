//===-- ISA32_LMInstrFormats.td - ISA32_LM Instruction Format Classes -----===//
//
// Clases base de formato de instrucción para la arquitectura ISA32_LM.
//
// Todas las instrucciones son estrictamente de 32 bits y alineadas a 4 bytes.
//
// Layout del campo de bits de 32 bits:
//   [31:29] Tipo   (3 bits) — modo de acceso a memoria o modificador
//   [28:24] OpCode (5 bits) — código de operación
//   [23:0]  Operandos (24 bits) — interpretados según el formato
//
// Formatos implementados:
//
//  Formato R  (Tres registros):
//    [31:29]=Tipo [28:24]=OpCode [23:20]=Rs1 [19:16]=Rs2 [15:12]=Rd [11:0]=0
//    Sintaxis: Op Rs1, Rs2, Rd
//
//  Formato I  (Registro + Inmediato):
//    [31:29]=Tipo [28:24]=OpCode [23:20]=Rd [19:16]=0(rsv) [15:0]=Imm16
//    Sintaxis: Op Rd, Inmediato
//
//  Formato III / Memoria (LOD/STR):
//    [31:29]=Tipo [28:24]=OpCode [23:20]=Rs1_Base [19:16]=Rs2_Dest [15:0]=Offset16
//    Sintaxis: Op Rs1_Base, Rs2_Dest, Offset
//
//  Formato IV (Saltos condicionales BRH):
//    [31:29]=Tipo [28:24]=OpCode [23:20]=Cond [19:16]=Rs1 [15:0]=0
//    Sintaxis: BRH Cond, Rs1
//
//  Formato None (Sin operandos):
//    [31:29]=Tipo [28:24]=OpCode [23:0]=0
//    Sintaxis: Op
//
//  Formato Reg1A (Un registro en [23:20]):
//    Sintaxis: Op Ra
//
//  Formato Reg1B (Un registro en [19:16]):
//    Sintaxis: Op Rb
//
//  Formato CYE/CYR (Banco especial ↔ banco general):
//    CYE: [23:20]=RC_esp [19:16]=RA_gp
//    CYR: [23:20]=RA_gp  [19:16]=RC_esp
//
// Referencia: assembler.py (Procesador-32-bits), función build_word().
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Clase base: toda instrucción ISA32_LM
//===----------------------------------------------------------------------===//

class ISA32LMInst<dag outs, dag ins, string asmstr, list<dag> pattern>
    : Instruction {
  field bits<32> Inst;

  let Namespace        = "ISA32_LM";
  let Size             = 4;           // 4 bytes = 32 bits por instrucción
  let OutOperandList   = outs;
  let InOperandList    = ins;
  let AsmString        = asmstr;
  let Pattern          = pattern;
  let DecoderNamespace = "ISA32_LM";

  // Campos compartidos por todos los formatos
  bits<3> Tipo;    // [31:29] — modo/modificador
  bits<5> OpCode;  // [28:24] — código de operación

  let Inst{31-29} = Tipo;
  let Inst{28-24} = OpCode;
  // Inst{23-0} lo establece cada subclase según su formato
}

// Pseudoinstrucción base (codegen solamente, no genera código máquina real)
class Pseudo<dag outs, dag ins, string asmstr, list<dag> pattern>
    : ISA32LMInst<outs, ins, asmstr, pattern> {
  let Tipo          = 0;
  let OpCode        = 0;
  let Inst          = 0;
  let isPseudo      = 1;
  let isCodeGenOnly = 1;
}


//===----------------------------------------------------------------------===//
// Helpers de codificación: valores de OpCode y Tipo
//===----------------------------------------------------------------------===//

class ISA32LMOpcode<bits<5> val> {
  bits<5> Value = val;
}

def OPC_NOP : ISA32LMOpcode<0b00000>;  //  0 — No operation
def OPC_HLT : ISA32LMOpcode<0b00001>;  //  1 — Halt
def OPC_ADD : ISA32LMOpcode<0b00010>;  //  2 — Add
def OPC_SUB : ISA32LMOpcode<0b00011>;  //  3 — Subtract
def OPC_MUL : ISA32LMOpcode<0b00100>;  //  4 — Multiply
def OPC_DIV : ISA32LMOpcode<0b00101>;  //  5 — Divide (unsigned)
def OPC_NOR : ISA32LMOpcode<0b00110>;  //  6 — NOR
def OPC_AND : ISA32LMOpcode<0b00111>;  //  7 — AND
def OPC_XOR : ISA32LMOpcode<0b01000>;  //  8 — XOR
def OPC_RSH : ISA32LMOpcode<0b01001>;  //  9 — Right shift (logical)
def OPC_LSH : ISA32LMOpcode<0b01010>;  // 10 — Left shift
def OPC_GOF : ISA32LMOpcode<0b01011>;  // 11 — Get overflow/carry flag
def OPC_LDI : ISA32LMOpcode<0b01100>;  // 12 — Load immediate
def OPC_ADI : ISA32LMOpcode<0b01101>;  // 13 — Add immediate (signed)
def OPC_JMP : ISA32LMOpcode<0b01110>;  // 14 — Jump (unconditional, indirect)
def OPC_BRH : ISA32LMOpcode<0b01111>;  // 15 — Branch conditional
def OPC_CAL : ISA32LMOpcode<0b10000>;  // 16 — Call subroutine
def OPC_RET : ISA32LMOpcode<0b10001>;  // 17 — Return from subroutine
def OPC_LOD : ISA32LMOpcode<0b10010>;  // 18 — Load from memory
def OPC_STR : ISA32LMOpcode<0b10011>;  // 19 — Store to memory
def OPC_CYE : ISA32LMOpcode<0b10100>;  // 20 — Copy special → general
def OPC_CYR : ISA32LMOpcode<0b10101>;  // 21 — Copy general → special
def OPC_SCL : ISA32LMOpcode<0b10110>;  // 22 — Syscall
def OPC_SRT : ISA32LMOpcode<0b10111>;  // 23 — Return to OS (from EPC)

// Campo Tipo [31:29]
class ISA32LMTipo<bits<3> val> {
  bits<3> Value = val;
}

def TIPO_NORMAL : ISA32LMTipo<0b000>; // 000 — modo normal / entero 32 bits
def TIPO_CHAR   : ISA32LMTipo<0b000>; // 000 - acceso a memoria de 8 bits
def TIPO_SHORT  : ISA32LMTipo<0b001>; // 001 — acceso a memoria de 16 bits
def TIPO_INT    : ISA32LMTipo<0b010>; // 010 — acceso a memoria de 32 bits
def TIPO_SLT    : ISA32LMTipo<0b011>; // 011 — modo silencioso / carga parte alta


//===----------------------------------------------------------------------===//
// Formato None — sin operandos
//
// Instrucciones: NOP, HLT, RET, SCL, SRT
// Encoding: [31:29]=Tipo [28:24]=OpCode [23:0]=0
//===----------------------------------------------------------------------===//
class Fmt_None<ISA32LMOpcode Opc, string asmstr, list<dag> pattern>
    : ISA32LMInst<(outs), (ins), asmstr, pattern> {
  let Tipo       = TIPO_NORMAL.Value;
  let OpCode     = Opc.Value;
  let Inst{23-0} = 0;
}


//===----------------------------------------------------------------------===//
// Formato R — tres registros (Rs1, Rs2, Rd)
//
// Instrucciones: ADD, SUB, MUL, DIV, NOR, AND, XOR, RSH, LSH
// Encoding: [23:20]=Rs1 [19:16]=Rs2 [15:12]=Rd [11:0]=0
// Sintaxis: Op Rs1, Rs2, Rd
//
// Nota: el ensamblador espera Op Rs1, Rs2, Rd (destino al final).
//===----------------------------------------------------------------------===//
class Fmt_R<ISA32LMOpcode Opc, bits<3> TipoVal,
            dag outs, dag ins, string asmstr, list<dag> pattern>
    : ISA32LMInst<outs, ins, asmstr, pattern> {
  bits<4> rs1;  // Ra: primer operando fuente  — campo [23:20]
  bits<4> rs2;  // Rb: segundo operando fuente — campo [19:16]
  bits<4> rd;   // Rc: operando destino        — campo [15:12]

  let Tipo   = TipoVal;
  let OpCode = Opc.Value;

  let Inst{23-20} = rs1;
  let Inst{19-16} = rs2;
  let Inst{15-12} = rd;
  let Inst{11-0}  = 0;   // bits reservados, siempre 0
}

// Alias con los nombres originales del ensamblador para compatibilidad interna
// (ra=Rs1, rb=Rs2, rc=Rd)
class Fmt_RRR<ISA32LMOpcode Opc, bits<3> TipoVal,
              dag outs, dag ins, string asmstr, list<dag> pattern>
    : Fmt_R<Opc, TipoVal, outs, ins, asmstr, pattern>;


//===----------------------------------------------------------------------===//
// Formato I — registro destino + inmediato de 16 bits
//
// Instrucciones: LDI, ADI (y variantes SLT)
// Encoding: [23:20]=Rd [19:16]=0(reservado) [15:0]=Imm16
// Sintaxis: Op Rd, Inmediato
//===----------------------------------------------------------------------===//
class Fmt_I<ISA32LMOpcode Opc, bits<3> TipoVal,
            dag outs, dag ins, string asmstr, list<dag> pattern>
    : ISA32LMInst<outs, ins, asmstr, pattern> {
  bits<4>  rd;    // Registro destino — campo [23:20]
  bits<16> imm16; // Inmediato 16 bits — campo [15:0]

  let Tipo   = TipoVal;
  let OpCode = Opc.Value;

  let Inst{23-20} = rd;
  let Inst{19-16} = 0;    // reservado, siempre 0
  let Inst{15-0}  = imm16;
}

// Alias Fmt_RI para compatibilidad con el nombre usado en InstrInfo.td
class Fmt_RI<ISA32LMOpcode Opc, bits<3> TipoVal,
             dag outs, dag ins, string asmstr, list<dag> pattern>
    : Fmt_I<Opc, TipoVal, outs, ins, asmstr, pattern>;


//===----------------------------------------------------------------------===//
// Formato III / Memoria — base + destino/fuente + offset
//
// Instrucciones: LOD, STR
// Encoding: [23:20]=Rs1_Base [19:16]=Rs2_Dest [15:0]=Offset16(signed)
// Sintaxis (LOD): LOD Rs1_Base, Rs2_Dest, Offset
//===----------------------------------------------------------------------===//
class Fmt_Mem<ISA32LMOpcode Opc, bits<3> TipoVal,
              dag outs, dag ins, string asmstr, list<dag> pattern>
    : ISA32LMInst<outs, ins, asmstr, pattern> {
  bits<4>  rs1_base;  // Registro base              — campo [23:20]
  bits<4>  rs2_dest;  // Registro dato/destino       — campo [19:16]
  bits<16> offset;    // Offset con signo de 16 bits — campo [15:0]

  let Tipo   = TipoVal;
  let OpCode = Opc.Value;

  let Inst{23-20} = rs1_base;
  let Inst{19-16} = rs2_dest;
  let Inst{15-0}  = offset;
}

// Alias Fmt_RRI para compatibilidad con el nombre del ensamblador
class Fmt_RRI<ISA32LMOpcode Opc, bits<3> TipoVal,
              dag outs, dag ins, string asmstr, list<dag> pattern>
    : Fmt_Mem<Opc, TipoVal, outs, ins, asmstr, pattern>;


//===----------------------------------------------------------------------===//
// Formato IV — Branch condicional (BRH)
//
// Encoding: [23:20]=Cond(4b) [19:16]=Rs1 [15:0]=0
// Sintaxis: BRH Cond, Rs1
//===----------------------------------------------------------------------===//
class Fmt_BRH<dag outs, dag ins, string asmstr, list<dag> pattern>
    : ISA32LMInst<outs, ins, asmstr, pattern> {
  bits<4> cond;  // Código de condición — campo [23:20]
  bits<4> rs1;   // Registro de destino — campo [19:16]

  let Tipo   = TIPO_NORMAL.Value;
  let OpCode = OPC_BRH.Value;

  let Inst{23-20} = cond;
  let Inst{19-16} = rs1;
  let Inst{15-0}  = 0;  // bits reservados
}

// Alias Fmt_CRB (condition + register B) para compatibilidad interna
class Fmt_CRB<dag outs, dag ins, string asmstr, list<dag> pattern>
    : Fmt_BRH<outs, ins, asmstr, pattern>;


//===----------------------------------------------------------------------===//
// Formato Reg1A — un registro en campo [23:20]
//
// Instrucción: GOF (Get Overflow Flag)
// Encoding: [23:20]=Ra [19:0]=0
// Sintaxis: GOF Ra
//===----------------------------------------------------------------------===//
class Fmt_RA<ISA32LMOpcode Opc, dag outs, dag ins,
             string asmstr, list<dag> pattern>
    : ISA32LMInst<outs, ins, asmstr, pattern> {
  bits<4> ra;  // Registro destino — campo [23:20]

  let Tipo   = TIPO_NORMAL.Value;
  let OpCode = Opc.Value;

  let Inst{23-20} = ra;
  let Inst{19-0}  = 0;
}


//===----------------------------------------------------------------------===//
// Formato Reg1B — un registro en campo [19:16]
//
// Instrucciones: JMP, CAL
// Encoding: [23:20]=0 [19:16]=Rb [15:0]=0
// Sintaxis: JMP/CAL Rb
//===----------------------------------------------------------------------===//
class Fmt_RB<ISA32LMOpcode Opc, dag outs, dag ins,
             string asmstr, list<dag> pattern>
    : ISA32LMInst<outs, ins, asmstr, pattern> {
  bits<4> rb;  // Registro fuente — campo [19:16]

  let Tipo   = TIPO_NORMAL.Value;
  let OpCode = Opc.Value;

  let Inst{23-20} = 0;
  let Inst{19-16} = rb;
  let Inst{15-0}  = 0;
}


//===----------------------------------------------------------------------===//
// Formatos CYE / CYR — transferencia banco especial ↔ banco general
//
// CYE: copia SR(rc_sp) → GPR(ra_gp)
//   Encoding: [23:20]=RC_especial [19:16]=RA_general [15:0]=0
//
// CYR: copia GPR(ra_gp) → SR(rc_sp)
//   Encoding: [23:20]=RA_general [19:16]=RC_especial [15:0]=0
//===----------------------------------------------------------------------===//
class Fmt_CYE<dag outs, dag ins, string asmstr, list<dag> pattern>
    : ISA32LMInst<outs, ins, asmstr, pattern> {
  bits<4> rc_sp;  // Registro especial fuente    — campo [23:20]
  bits<4> ra_gp;  // Registro general destino    — campo [19:16]

  let Tipo   = TIPO_NORMAL.Value;
  let OpCode = OPC_CYE.Value;

  let Inst{23-20} = rc_sp;
  let Inst{19-16} = ra_gp;
  let Inst{15-0}  = 0;
}

class Fmt_CYR<dag outs, dag ins, string asmstr, list<dag> pattern>
    : ISA32LMInst<outs, ins, asmstr, pattern> {
  bits<4> ra_gp;  // Registro general fuente     — campo [23:20]
  bits<4> rc_sp;  // Registro especial destino   — campo [19:16]

  let Tipo   = TIPO_NORMAL.Value;
  let OpCode = OPC_CYR.Value;

  let Inst{23-20} = ra_gp;
  let Inst{19-16} = rc_sp;
  let Inst{15-0}  = 0;
}
