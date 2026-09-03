#===-- llvm/lib/Target/ISA32_LM/CMakeLists.txt ----------------------------===//
#
# Sistema de compilación para el backend ISA32_LM de LLVM.
#
# Arquitectura: RISC custom de 32 bits, Little-Endian.
#
# Archivos TableGen que se compilan desde ISA32_LM.td:
#   ISA32_LMGenRegisterInfo.inc  — enumeraciones y descriptores de registros
#   ISA32_LMGenInstrInfo.inc     — descriptores de instrucciones
#   ISA32_LMGenDAGISel.inc       — selector de instrucciones DAG
#   ISA32_LMGenSubtargetInfo.inc — información de subtarget / procesadores
#   ISA32_LMGenAsmWriter.inc     — impresor de ensamblador (InstPrinter)
#   ISA32_LMGenAsmMatcher.inc    — analizador de ensamblador (AsmParser)
#   ISA32_LMGenDisassemblerTables.inc — tablas del desensamblador
#   ISA32_LMGenMCCodeEmitter.inc — emisor de código máquina (MC layer)
#
#===----------------------------------------------------------------------===//

add_llvm_component_group(ISA32_LM)

set(LLVM_TARGET_DEFINITIONS ISA32_LM.td)

# ── Tablas generadas por TableGen ─────────────────────────────────────────

# Convención de llamadas (CallingConv)
tablegen(LLVM ISA32_LMGenCallingConv.inc    -gen-callingconv)

# Información de registros (enumeraciones, descriptores, DwarfRegNum, etc.)
tablegen(LLVM ISA32_LMGenRegisterInfo.inc   -gen-register-info)

# Descriptores de instrucciones (InstrInfo)
tablegen(LLVM ISA32_LMGenInstrInfo.inc      -gen-instr-info)

# Selector de instrucciones DAG (ISelDAGToDAG)
tablegen(LLVM ISA32_LMGenDAGISel.inc        -gen-dag-isel)

# Información de subtarget y procesadores
tablegen(LLVM ISA32_LMGenSubtargetInfo.inc  -gen-subtarget)

# Escritor de ensamblador (InstPrinter / AsmWriter)
tablegen(LLVM ISA32_LMGenAsmWriter.inc      -gen-asm-writer)

# Analizador de ensamblador (AsmParser / AsmMatcher)
tablegen(LLVM ISA32_LMGenAsmMatcher.inc     -gen-asm-matcher)

# Tablas del desensamblador
tablegen(LLVM ISA32_LMGenDisassemblerTables.inc -gen-disassembler)

# Emisor de código máquina (MC layer)
tablegen(LLVM ISA32_LMGenMCCodeEmitter.inc  -gen-emitter)

add_public_tablegen_target(ISA32_LMCommonTableGen)

# ── Biblioteca principal del backend (CodeGen) ────────────────────────────

add_llvm_target(ISA32_LMCodeGen
  ISA32_LMAsmPrinter.cpp
  ISA32_LMFrameLowering.cpp
  ISA32_LMISelDAGToDAG.cpp
  ISA32_LMISelLowering.cpp
  ISA32_LMInstrInfo.cpp
  ISA32_LMMCInstLower.cpp
  ISA32_LMRegisterInfo.cpp
  ISA32_LMSubtarget.cpp
  ISA32_LMTargetMachine.cpp

  LINK_COMPONENTS
    Analysis
    AsmPrinter
    ISA32_LMDesc
    ISA32_LMInfo
    CodeGen
    CodeGenTypes
    Core
    MC
    SelectionDAG
    Support
    Target
    TargetParser
    TransformUtils

  ADD_TO_COMPONENT
    ISA32_LM
)

# ── Subdirectorios ─────────────────────────────────────────────────────────
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
# add_subdirectory(AsmParser)
# add_subdirectory(Disassembler)

