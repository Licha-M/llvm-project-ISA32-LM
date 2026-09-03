//===-- ISA32_LMISelLowering.h - ISA32_LM DAG Lowering Interface -*- C++ -*-===//
//
// Interfaz de TargetLowering para la arquitectura ISA32_LM.
//
// Responsabilidades:
//   · Legalización de tipos: Legalizar i32, Expandir i64 a pares de 32 bits.
//   · Convención de llamadas: Pasaje de argumentos en R1-R7 / pila (Stack-Up).
//   · Lowering personalizado de ISD::GlobalAddress, ISD::BR_CC, ISD::SELECT_CC.
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ISA32_LM_ISA32_LMISELLOWERING_H
#define LLVM_LIB_TARGET_ISA32_LM_ISA32_LMISELLOWERING_H

#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {

namespace ISA32_LMISD {
enum NodeType : unsigned {
  FIRST_NUMBER = ISD::BUILTIN_OP_END,
  CALL,
  RET_GLUE,
  BR_CC,
  SELECT_CC,
  Wrapper,
  GOF,
  HLT,
  SCL,
  SRT
};
} // namespace ISA32_LMISD

class ISA32_LMSubtarget;

class ISA32_LMTargetLowering : public TargetLowering {
  const ISA32_LMSubtarget &Subtarget;

public:
  explicit ISA32_LMTargetLowering(const TargetMachine &TM,
                                  const ISA32_LMSubtarget &STI);

  // Devuelve el nombre legible para nodos SDNode personalizados
  const char *getTargetNodeName(unsigned Opcode) const override;

  // Lowering personalizado de operaciones registradas con Custom
  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

  // Lowering de Convención de Llamadas (C Calling Convention)
  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool IsVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &DL, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, const SDLoc &DL,
                      SelectionDAG &DAG) const override;

  SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
                    SmallVectorImpl<SDValue> &InVals) const override;

  MachineBasicBlock *
  EmitInstrWithCustomInserter(MachineInstr &MI,
                              MachineBasicBlock *BB) const override;

private:
  SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerBR_CC(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_ISA32_LM_ISA32_LMISELLOWERING_H
