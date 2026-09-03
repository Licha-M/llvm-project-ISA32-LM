//===-- ISA32_LMISelDAGToDAG.cpp - Instruction Selector para ISA32_LM -----===//
#include "ISA32_LM.h"
#include "ISA32_LMTargetMachine.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define GET_REGINFO_ENUM
#include "ISA32_LMGenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#include "ISA32_LMGenInstrInfo.inc"

using namespace llvm;

#define DEBUG_TYPE "isa32_lm-isel"
#define PASS_NAME "ISA32_LM DAG->DAG Pattern Instruction Selection"

namespace {

class ISA32_LMDAGToDAGISel : public SelectionDAGISel {
public:
  ISA32_LMDAGToDAGISel() = delete;

  explicit ISA32_LMDAGToDAGISel(ISA32_LMTargetMachine &TM,
                                CodeGenOptLevel OptLevel)
      : SelectionDAGISel(TM, OptLevel) {}

  void Select(SDNode *N) override;

// TableGen genera las DECLARACIONES de las funciones (ej: SelectCode) aquí
#define GET_DAGISEL_DECL ISA32_LMDAGToDAGISel
#include "ISA32_LMGenDAGISel.inc"
};

class ISA32_LMISelDAGToDAGLegacy : public SelectionDAGISelLegacy {
public:
  static char ID;
  explicit ISA32_LMISelDAGToDAGLegacy(ISA32_LMTargetMachine &TM,
                                      CodeGenOptLevel OptLevel)
      : SelectionDAGISelLegacy(
            ID, std::make_unique<ISA32_LMDAGToDAGISel>(TM, OptLevel)) {}

  StringRef getPassName() const override { return PASS_NAME; }
};

} // namespace

char ISA32_LMISelDAGToDAGLegacy::ID = 0;

// TableGen genera los CUERPOS de las funciones cualificadas aquí (fuera de la
// clase)
#define GET_DAGISEL_BODY ISA32_LMDAGToDAGISel
#include "ISA32_LMGenDAGISel.inc"

// Declaración en el espacio de nombres llvm requerida para la macro
// INITIALIZE_PASS
namespace llvm {
void initializeISA32_LMISelDAGToDAGLegacyPass(PassRegistry &);
}

INITIALIZE_PASS(ISA32_LMISelDAGToDAGLegacy, DEBUG_TYPE, PASS_NAME, false, false)

void ISA32_LMDAGToDAGISel::Select(SDNode *N) {
  if (N->isMachineOpcode()) {
    LLVM_DEBUG(dbgs() << "== "; N->dump(CurDAG); dbgs() << "\n");
    return;
  }

  // Atrapamos el nodo de FrameIndex (variables guardadas en la pila)
  if (N->getOpcode() == ISD::FrameIndex) {
    SDLoc DL(N);
    int FI = cast<FrameIndexSDNode>(N)->getIndex();

    // 1. Convertimos a un índice específico de tu máquina
    SDValue TFI = CurDAG->getTargetFrameIndex(FI, MVT::i32);

    // 2. Traemos el Stack Pointer (R14) y el registro Zero (R0)
    SDValue R14 = CurDAG->getRegister(ISA32_LM::R14, MVT::i32);
    SDValue R0 = CurDAG->getRegister(ISA32_LM::R0, MVT::i32);

    // 3. Creamos la instrucción: Dest = ADD R14, R0 (Copia el SP a un registro
    // virtual)
    SDNode *Add =
        CurDAG->getMachineNode(ISA32_LM::ADD_RRR, DL, MVT::i32, R14, R0);

    // 4. Creamos la instrucción: Dest = ADI_SLT Dest, Offset (Le suma el offset
    // de la pila)
    SDNode *Adi = CurDAG->getMachineNode(ISA32_LM::ADI_SLT, DL, MVT::i32,
                                         SDValue(Add, 0), TFI);

    // 5. Reemplazamos el nodo abstracto original por nuestra instrucción real
    ReplaceNode(N, Adi);
    return;
  }

  // Si no es un FrameIndex, dejamos que LLVM use los patrones automáticos de
  // TableGen
  SelectCode(N);
}

FunctionPass *llvm::createISA32_LMISelDagLegacyPass(ISA32_LMTargetMachine &TM,
                                                    CodeGenOptLevel OptLevel) {
  return new ISA32_LMISelDAGToDAGLegacy(TM, OptLevel);
}