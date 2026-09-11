//===-- ISA32_LMISelLowering.cpp - ISA32_LM DAG Lowering Implementation --===//
//
// Implementación de TargetLowering para la arquitectura ISA32_LM.
//
// Reglas clave:
//   1. Legalización de tipos: MVT::i32 es el único tipo entero legal.
//      MVT::i64 es marcado como EXPAND (Expand), obligando al compilador
//      a dividir tipos y operaciones de 64 bits en pares de registros de 32
//      bits.
//   2. Pila: El Stack Pointer es R14. La pila crece hacia ARRIBA (Low to High).
//   3. Convención de llamadas: Usa CC_ISA32_LM y RetCC_ISA32_LM.
//===----------------------------------------------------------------------===//

#include "ISA32_LMISelLowering.h"
#include "ISA32_LM.h"
#include "ISA32_LMRegisterInfo.h"
#include "ISA32_LMSubtarget.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/Support/ErrorHandling.h"

#include "llvm/CodeGen/TargetInstrInfo.h"

using namespace llvm;

#define DEBUG_TYPE "isa32_lm-lower"

// Enum de registros (R0..R15, SR0..SR15). Necesario ANTES del include de
// CallingConv.inc, porque CC_ISA32_LM/RetCC_ISA32_LM referencian los
// nombres de registro (R1, R2, ...) directamente en su cuerpo generado.
#define GET_REGINFO_ENUM
#include "ISA32_LMGenRegisterInfo.inc"

// Generado por TableGen a partir de ISA32_LMCallingConv.td
#define GET_CALLING_CONV_IMPL
#include "ISA32_LMGenCallingConv.inc"

#define GET_INSTRINFO_ENUM
#include "ISA32_LMGenInstrInfo.inc"

//===----------------------------------------------------------------------===//
// Helper: Traducción de ISD::CondCode a código de condición de 4 bits ISA32_LM
//===----------------------------------------------------------------------===//
//
// Códigos de condición de hardware ISA32_LM (4 bits, campo Cond de BRH):
//   0000 = 0 (EQ)  - Zero flag activo
//   0001 = 1 (NE)  - Not Zero flag activo
//   0010 = 2 (N)   - Negative flag activo
//   0011 = 3 (NN)  - Not Negative flag activo
//   0100 = 4 (C)   - Carry flag activo
//   0101 = 5 (NC)  - Not Carry flag activo
//   0110 = 6 (OV)  - Overflow flag activo
//   0111 = 7 (NOV) - Not Overflow flag activo
//
// Para comparaciones que no tienen flag directo en hardware (GT, LE, UGT, ULE),
// se resuelven intercambiando los operandos (NeedsSwap = true) y utilizando el
// flag equivalente (ej. Rs1 > Rs2 <=> Rs2 < Rs1).
static unsigned translateISA32LMCC(ISD::CondCode CC, bool &NeedsSwap) {
  NeedsSwap = false;
  switch (CC) {
  default:
    llvm_unreachable("CondCode no soportado en ISA32_LM");
  case ISD::SETEQ:
    return 0; // EQ (0000)
  case ISD::SETNE:
    return 1; // NE (0001)
  case ISD::SETLT:
    return 2; // N  (0010) - Rs1 - Rs2 < 0 con signo
  case ISD::SETGE:
    return 3; // NN (0011) - Rs1 - Rs2 >= 0 con signo
  case ISD::SETGT:
    NeedsSwap = true;
    return 2; // N  (0010) - Rs2 < Rs1 con signo
  case ISD::SETLE:
    NeedsSwap = true;
    return 3; // NN (0011) - Rs2 >= Rs1 con signo
  case ISD::SETULT:
    return 4; // C  (0100) - Rs1 < Rs2 sin signo
  case ISD::SETUGE:
    return 5; // NC (0101) - Rs1 >= Rs2 sin signo
  case ISD::SETUGT:
    NeedsSwap = true;
    return 4; // C  (0100) - Rs2 < Rs1 sin signo
  case ISD::SETULE:
    NeedsSwap = true;
    return 5; // NC (0101) - Rs2 >= Rs1 sin signo
  }
}

ISA32_LMTargetLowering::ISA32_LMTargetLowering(const TargetMachine &TM,
                                               const ISA32_LMSubtarget &STI)
    : TargetLowering(TM, STI), Subtarget(STI) {

  // ── Registrar clase de registros legales (32 bits) ──────────────────────
  addRegisterClass(MVT::i32, &ISA32_LM::GPR32RegClass);

  // Calcular la tabla de preferencia de tipos
  computeRegisterProperties(STI.getRegisterInfo());

  // ── Registro Stack Pointer (R14) ─────────────────────────────────────────
  setStackPointerRegisterToSaveRestore(ISA32_LM::R14);

  // ── Modos Booleanos ──────────────────────────────────────────────────────
  setBooleanContents(ZeroOrOneBooleanContent);

  // ── Legalización de i64 (EXPANDIR i64 a pares de 32 bits) ────────────────
  // Al ser una arquitectura pura de 32 bits, la legalización de tipos fuerza
  // la división de i64 en (low32, high32).
  setOperationAction(ISD::ADD, MVT::i64, Expand);
  setOperationAction(ISD::SUB, MVT::i64, Expand);
  setOperationAction(ISD::MUL, MVT::i64, Expand);
  setOperationAction(ISD::SDIV, MVT::i64, Expand);
  setOperationAction(ISD::UDIV, MVT::i64, Expand);
  setOperationAction(ISD::SREM, MVT::i64, Expand);
  setOperationAction(ISD::UREM, MVT::i64, Expand);
  setOperationAction(ISD::SHL, MVT::i64, Expand);
  setOperationAction(ISD::SRL, MVT::i64, Expand);
  setOperationAction(ISD::SRA, MVT::i64, Expand);
  setOperationAction(ISD::ROTL, MVT::i64, Expand);
  setOperationAction(ISD::ROTR, MVT::i64, Expand);
  setOperationAction(ISD::BSWAP, MVT::i64, Expand);
  setOperationAction(ISD::CTPOP, MVT::i64, Expand);
  setOperationAction(ISD::CTTZ, MVT::i64, Expand);
  setOperationAction(ISD::CTLZ, MVT::i64, Expand);

  // ── Operaciones con Lowering personalizado en 32 bits ───────────────────
  setOperationAction(ISD::GlobalAddress, MVT::i32, Custom);
  setOperationAction(ISD::BR_CC, MVT::i32, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::i32, Custom);
  setOperationAction(ISD::SETCC, MVT::i32, Expand);

  // Expansiones estándar de 32 bits para operaciones no soportadas directamente
  // por HW
  setOperationAction(ISD::SDIV, MVT::i32, Expand);
  setOperationAction(ISD::SREM, MVT::i32, Expand);
  setOperationAction(ISD::UREM, MVT::i32, Expand);
  setOperationAction(ISD::BR_JT, MVT::Other, Expand);
  setOperationAction(ISD::BRIND, MVT::Other, Expand);
  setOperationAction(ISD::BRCOND, MVT::Other, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16, Expand);

  // ── Legalización de Cargas Extendidas (Solución al error) ────────────────
  // Promueve las cargas con extensión de cero/signo de i1 a tipos soportados
  // por HW
  setLoadExtAction(ISD::ZEXTLOAD, MVT::i32, MVT::i1, Promote);
  setLoadExtAction(ISD::SEXTLOAD, MVT::i32, MVT::i1, Promote);
  setLoadExtAction(ISD::EXTLOAD, MVT::i32, MVT::i1, Promote);
  // ── Operaciones Atómicas ─────────────────────────────────────────────────
  setMaxAtomicSizeInBitsSupported(0);
  setOperationAction(ISD::ATOMIC_LOAD, MVT::i32, Expand);
  setOperationAction(ISD::ATOMIC_STORE, MVT::i32, Expand);
}

const char *ISA32_LMTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch ((ISA32_LMISD::NodeType)Opcode) {
  case ISA32_LMISD::FIRST_NUMBER:
    break;
  case ISA32_LMISD::CALL:
    return "ISA32_LMISD::CALL";
  case ISA32_LMISD::RET_GLUE:
    return "ISA32_LMISD::RET_GLUE";
  case ISA32_LMISD::BR_CC:
    return "ISA32_LMISD::BR_CC";
  case ISA32_LMISD::SELECT_CC:
    return "ISA32_LMISD::SELECT_CC";
  case ISA32_LMISD::Wrapper:
    return "ISA32_LMISD::Wrapper";
  case ISA32_LMISD::GOF:
    return "ISA32_LMISD::GOF";
  case ISA32_LMISD::HLT:
    return "ISA32_LMISD::HLT";
  case ISA32_LMISD::SCL:
    return "ISA32_LMISD::SCL";
  case ISA32_LMISD::SRT:
    return "ISA32_LMISD::SRT";
  }
  return nullptr;
}

SDValue ISA32_LMTargetLowering::LowerOperation(SDValue Op,
                                               SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  case ISD::GlobalAddress:
    return LowerGlobalAddress(Op, DAG);
  case ISD::BR_CC:
    return LowerBR_CC(Op, DAG);
  case ISD::SELECT_CC:
    return LowerSELECT_CC(Op, DAG);
  default:
    llvm_unreachable("Opcode de operación no implementado en LowerOperation");
  }
}

SDValue ISA32_LMTargetLowering::LowerGlobalAddress(SDValue Op,
                                                   SelectionDAG &DAG) const {
  SDLoc DL(Op);
  const GlobalAddressSDNode *GA = cast<GlobalAddressSDNode>(Op);
  SDValue TargetAddr = DAG.getTargetGlobalAddress(GA->getGlobal(), DL, MVT::i32,
                                                  GA->getOffset());
  return DAG.getNode(ISA32_LMISD::Wrapper, DL, MVT::i32, TargetAddr);
}

SDValue ISA32_LMTargetLowering::LowerBR_CC(SDValue Op,
                                           SelectionDAG &DAG) const {
  SDValue Chain = Op.getOperand(0);
  ISD::CondCode CCVal = cast<CondCodeSDNode>(Op.getOperand(1))->get();
  SDValue LHS = Op.getOperand(2);
  SDValue RHS = Op.getOperand(3);
  SDValue Dest = Op.getOperand(4);
  SDLoc DL(Op);

  bool NeedsSwap = false;
  unsigned TargetCC = translateISA32LMCC(CCVal, NeedsSwap);
  if (NeedsSwap)
    std::swap(LHS, RHS);

  SDValue TargetCCVal = DAG.getTargetConstant(TargetCC, DL, MVT::i32);
  return DAG.getNode(ISA32_LMISD::BR_CC, DL, Op.getValueType(), Chain, LHS, RHS,
                     TargetCCVal, Dest);
}

SDValue ISA32_LMTargetLowering::LowerSELECT_CC(SDValue Op,
                                               SelectionDAG &DAG) const {
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue TrueVal = Op.getOperand(2);
  SDValue FalseVal = Op.getOperand(3);
  ISD::CondCode CCVal = cast<CondCodeSDNode>(Op.getOperand(4))->get();
  SDLoc DL(Op);

  bool NeedsSwap = false;
  unsigned TargetCC = translateISA32LMCC(CCVal, NeedsSwap);
  if (NeedsSwap)
    std::swap(LHS, RHS);

  SDValue TargetCCVal = DAG.getTargetConstant(TargetCC, DL, MVT::i32);
  return DAG.getNode(ISA32_LMISD::SELECT_CC, DL, Op.getValueType(), LHS, RHS,
                     TargetCCVal, TrueVal, FalseVal);
}

//===----------------------------------------------------------------------===//
// EmitInstrWithCustomInserter — Expansión del pseudo SELECT_CC en diamantes
//===----------------------------------------------------------------------===//

MachineBasicBlock *ISA32_LMTargetLowering::EmitInstrWithCustomInserter(
    MachineInstr &MI, MachineBasicBlock *BB) const {
  const TargetInstrInfo &TII = *Subtarget.getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();

  assert(MI.getOpcode() == ISA32_LM::SELECT_CC &&
         "Instrucción no esperada en Custom Inserter");

  Register DstReg = MI.getOperand(0).getReg();
  Register LHSReg = MI.getOperand(1).getReg();
  Register RHSReg = MI.getOperand(2).getReg();
  unsigned CCCode = MI.getOperand(3).getImm();
  Register TrueReg = MI.getOperand(4).getReg();
  Register FalseReg = MI.getOperand(5).getReg();

  MachineFunction *F = BB->getParent();
  MachineFunction::iterator It = ++BB->getIterator();

  MachineBasicBlock *thisMBB = BB;
  MachineBasicBlock *copy0MBB = F->CreateMachineBasicBlock(BB->getBasicBlock());
  MachineBasicBlock *sinkMBB = F->CreateMachineBasicBlock(BB->getBasicBlock());

  F->insert(It, copy0MBB);
  F->insert(It, sinkMBB);

  // Mover el resto de instrucciones después de MI de thisMBB a sinkMBB
  sinkMBB->splice(sinkMBB->begin(), thisMBB,
                  std::next(MachineBasicBlock::iterator(MI)), thisMBB->end());
  sinkMBB->transferSuccessorsAndUpdatePHIs(thisMBB);

  // Configurar los sucesores de los bloques
  thisMBB->addSuccessor(copy0MBB);
  thisMBB->addSuccessor(sinkMBB);

  // Insertar el branch condicional en thisMBB: si la condición se cumple, salta
  // a copy0MBB
  BuildMI(thisMBB, DL, TII.get(ISA32_LM::BRH32_PSEUDO))
      .addReg(LHSReg)
      .addReg(RHSReg)
      .addImm(CCCode)
      .addMBB(copy0MBB);

  copy0MBB->addSuccessor(sinkMBB);

  // Insertar el nodo PHI en sinkMBB para elegir el valor final
  BuildMI(*sinkMBB, sinkMBB->begin(), DL, TII.get(TargetOpcode::PHI), DstReg)
      .addReg(TrueReg)
      .addMBB(copy0MBB)
      .addReg(FalseReg)
      .addMBB(thisMBB);

  MI.eraseFromParent();
  return sinkMBB;
}

//===----------------------------------------------------------------------===//
// LowerFormalArguments — Recibir parámetros de entrada según CC_ISA32_LM
//===----------------------------------------------------------------------===//

SDValue ISA32_LMTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {

  MachineFunction &MF = DAG.getMachineFunction();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());

  CCInfo.AnalyzeFormalArguments(Ins, CC_ISA32_LM);

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];

    if (VA.isRegLoc()) {
      EVT RegVT = VA.getLocVT();
      Register VReg = RegInfo.createVirtualRegister(&ISA32_LM::GPRRegClass);
      RegInfo.addLiveIn(VA.getLocReg(), VReg);
      SDValue ArgValue = DAG.getCopyFromReg(Chain, DL, VReg, RegVT);
      InVals.push_back(ArgValue);
    } else {
      assert(VA.isMemLoc() &&
             "La ubicación del argumento debe ser registro o memoria");
      // La pila crece hacia ARRIBA (Stack Grows UP)
      int FI =
          MF.getFrameInfo().CreateFixedObject(4, VA.getLocMemOffset(), true);
      SDValue FIN = DAG.getFrameIndex(FI, MVT::i32);
      SDValue ArgValue = DAG.getLoad(MVT::i32, DL, Chain, FIN,
                                     MachinePointerInfo::getFixedStack(MF, FI));
      InVals.push_back(ArgValue);
    }
  }

  return Chain;
}

//===----------------------------------------------------------------------===//
// LowerReturn — Retornar valores de función según RetCC_ISA32_LM
//===----------------------------------------------------------------------===//

SDValue
ISA32_LMTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                    bool IsVarArg,
                                    const SmallVectorImpl<ISD::OutputArg> &Outs,
                                    const SmallVectorImpl<SDValue> &OutVals,
                                    const SDLoc &DL, SelectionDAG &DAG) const {

  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  CCInfo.AnalyzeReturn(Outs, RetCC_ISA32_LM);

  SDValue Glue;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  for (unsigned i = 0, e = RVLocs.size(); i != e; ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "El valor de retorno debe estar en un registro");

    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), OutVals[i], Glue);
    Glue = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain;

  if (Glue.getNode())
    RetOps.push_back(Glue);

  return DAG.getNode(ISA32_LMISD::RET_GLUE, DL, MVT::Other, RetOps);
}

//===----------------------------------------------------------------------===//
// LowerCall — Realizar llamadas a subrutinas (CAL)
//===----------------------------------------------------------------------===//

SDValue
ISA32_LMTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                  SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SDLoc DL = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  CallingConv::ID CallConv = CLI.CallConv;
  bool IsVarArg = CLI.IsVarArg;

  MachineFunction &MF = DAG.getMachineFunction();

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeCallOperands(Outs, CC_ISA32_LM);

  unsigned NumBytes = CCInfo.getAlignedCallFrameSize();

  Chain = DAG.getCALLSEQ_START(Chain, NumBytes, 0, DL);

  SmallVector<std::pair<unsigned, SDValue>, 8> RegsToPass;
  SmallVector<SDValue, 8> MemOpChains;

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    SDValue Arg = OutVals[i];

    if (VA.isRegLoc()) {
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
    } else {
      assert(VA.isMemLoc());
      SDValue StackPtr = DAG.getRegister(ISA32_LM::R14, MVT::i32);
      SDValue PtrOff = DAG.getIntPtrConstant(VA.getLocMemOffset(), DL);
      SDValue Addr = DAG.getNode(ISD::ADD, DL, MVT::i32, StackPtr, PtrOff);
      MemOpChains.push_back(
          DAG.getStore(Chain, DL, Arg, Addr, MachinePointerInfo()));
    }
  }

  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, MemOpChains);

  SDValue Glue;
  for (auto &Reg : RegsToPass) {
    Chain = DAG.getCopyToReg(Chain, DL, Reg.first, Reg.second, Glue);
    Glue = Chain.getValue(1);
  }

  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee))
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), DL, MVT::i32,
                                        G->getOffset());
  else if (ExternalSymbolSDNode *S = dyn_cast<ExternalSymbolSDNode>(Callee))
    Callee = DAG.getTargetExternalSymbol(S->getSymbol(), MVT::i32);

  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);

  for (auto &Reg : RegsToPass)
    Ops.push_back(DAG.getRegister(Reg.first, Reg.second.getValueType()));

  const ISA32_LMRegisterInfo *TRI = Subtarget.getRegisterInfo();
  const uint32_t *Mask = TRI->getCallPreservedMask(MF, CallConv);
  Ops.push_back(DAG.getRegisterMask(Mask));

  if (Glue.getNode())
    Ops.push_back(Glue);

  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  Chain = DAG.getNode(ISA32_LMISD::CALL, DL, NodeTys, Ops);
  Glue = Chain.getValue(1);

  Chain = DAG.getCALLSEQ_END(Chain, NumBytes, 0, Glue, DL);
  Glue = Chain.getValue(1);

  // Recibir valor de retorno
  SmallVector<CCValAssign, 16> RVLocs;
  CCState RetCCInfo(CallConv, IsVarArg, MF, RVLocs, *DAG.getContext());
  RetCCInfo.AnalyzeCallResult(Ins, RetCC_ISA32_LM);

  for (unsigned i = 0, e = RVLocs.size(); i != e; ++i) {
    CCValAssign &VA = RVLocs[i];
    Chain = DAG.getCopyFromReg(Chain, DL, VA.getLocReg(), VA.getLocVT(), Glue)
                .getValue(1);
    Glue = Chain.getValue(2);
    InVals.push_back(Chain.getValue(0));
  }

  return Chain;
}