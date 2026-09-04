//===-- ISA32_LMAsmParser.cpp - Parse ISA32_LM assembly to MCInst
//-----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// MCTargetAsmParser mínimo y extensible para la arquitectura ISA32_LM.
// Soporta mnemónicos sin operandos (ej. HLT, NOP, RET, SCL, SRT),
// instrucciones con operandos de registro/inmediato y soporte de inline asm.
//
//===----------------------------------------------------------------------===//

#include "../MCTargetDesc/ISA32_LMMCTargetDesc.h"
#include "../TargetInfo/ISA32_LMTargetInfo.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCParser/AsmLexer.h"
#include "llvm/MC/MCParser/MCAsmParser.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/SMLoc.h"

using namespace llvm;

#define DEBUG_TYPE "isa32_lm-asm-parser"

namespace {

class ISA32_LMOperand : public MCParsedAsmOperand {
  enum KindTy {
    k_Token,
    k_Register,
    k_Immediate,
  } Kind;

  SMLoc StartLoc, EndLoc;

  struct TokenOp {
    const char *Data;
    unsigned Length;
  };

  struct RegOp {
    MCRegister RegNum;
  };

  struct ImmOp {
    const MCExpr *Val;
  };

  union {
    struct TokenOp Tok;
    struct RegOp Reg;
    struct ImmOp Imm;
  };

public:
  ISA32_LMOperand(KindTy K) : Kind(K) {}

  bool isToken() const override { return Kind == k_Token; }
  bool isReg() const override { return Kind == k_Register; }
  bool isImm() const override { return Kind == k_Immediate; }
  bool isMem() const override { return false; }

  StringRef getToken() const {
    assert(Kind == k_Token && "Invalid access!");
    return StringRef(Tok.Data, Tok.Length);
  }

  MCRegister getReg() const override {
    assert(Kind == k_Register && "Invalid access!");
    return Reg.RegNum;
  }

  const MCExpr *getImm() const {
    assert(Kind == k_Immediate && "Invalid access!");
    return Imm.Val;
  }

  SMLoc getStartLoc() const override { return StartLoc; }
  SMLoc getEndLoc() const override { return EndLoc; }

  void print(raw_ostream &OS, const MCAsmInfo &MAI) const override {
    switch (Kind) {
    case k_Token:
      OS << "Token: '" << getToken() << "'";
      break;
    case k_Register:
      OS << "Reg: " << getReg().id();
      break;
    case k_Immediate:
      OS << "Imm: " << getImm();
      break;
    }
  }

  void addRegOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    Inst.addOperand(MCOperand::createReg(getReg()));
  }

  void addImmOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    addExpr(Inst, getImm());
  }

  void addExpr(MCInst &Inst, const MCExpr *Expr) const {
    if (!Expr)
      Inst.addOperand(MCOperand::createImm(0));
    else if (const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(Expr))
      Inst.addOperand(MCOperand::createImm(CE->getValue()));
    else
      Inst.addOperand(MCOperand::createExpr(Expr));
  }

  static std::unique_ptr<ISA32_LMOperand> CreateToken(StringRef Str, SMLoc S) {
    auto Op = std::make_unique<ISA32_LMOperand>(k_Token);
    Op->Tok.Data = Str.data();
    Op->Tok.Length = Str.size();
    Op->StartLoc = S;
    Op->EndLoc = S;
    return Op;
  }

  static std::unique_ptr<ISA32_LMOperand> CreateReg(MCRegister RegNum, SMLoc S,
                                                    SMLoc E) {
    auto Op = std::make_unique<ISA32_LMOperand>(k_Register);
    Op->Reg.RegNum = RegNum;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  static std::unique_ptr<ISA32_LMOperand> CreateImm(const MCExpr *Val, SMLoc S,
                                                    SMLoc E) {
    auto Op = std::make_unique<ISA32_LMOperand>(k_Immediate);
    Op->Imm.Val = Val;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }
};

class ISA32_LMAsmParser : public MCTargetAsmParser {
  MCAsmParser &Parser;

#define GET_ASSEMBLER_HEADER
#include "ISA32_LMGenAsmMatcher.inc"

  bool matchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                               OperandVector &Operands, MCStreamer &Out,
                               uint64_t &ErrorInfo,
                               bool MatchingInlineAsm) override;

  bool parseRegister(MCRegister &Reg, SMLoc &StartLoc, SMLoc &EndLoc) override;
  ParseStatus tryParseRegister(MCRegister &Reg, SMLoc &StartLoc,
                               SMLoc &EndLoc) override;

  bool parseInstruction(ParseInstructionInfo &Info, StringRef Name,
                        SMLoc NameLoc, OperandVector &Operands) override;

  ParseStatus parseDirective(AsmToken DirectiveID) override {
    return ParseStatus::NoMatch;
  }

  bool parseOperand(OperandVector &Operands, StringRef Mnemonic);
  ParseStatus parseConditionCode(OperandVector &Operands);

public:
  ISA32_LMAsmParser(const MCSubtargetInfo &STI, MCAsmParser &Parser,
                    const MCInstrInfo &MII)
      : MCTargetAsmParser(STI, MII), Parser(Parser) {
    setAvailableFeatures(ComputeAvailableFeatures(STI.getFeatureBits()));
  }

  MCAsmParser &getParser() const { return Parser; }
  AsmLexer &getLexer() const { return Parser.getLexer(); }
};

} // end anonymous namespace

#define GET_MATCHER_IMPLEMENTATION
#define GET_REGISTER_MATCHER
#define GET_MNEMONIC_SPELL_CHECKER
#include "ISA32_LMGenAsmMatcher.inc"

bool ISA32_LMAsmParser::matchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                                                OperandVector &Operands,
                                                MCStreamer &Out,
                                                uint64_t &ErrorInfo,
                                                bool MatchingInlineAsm) {
  MCInst Inst;
  unsigned MatchResult =
      MatchInstructionImpl(Operands, Inst, ErrorInfo, MatchingInlineAsm);

  switch (MatchResult) {
  case Match_Success:
    Inst.setLoc(IDLoc);
    Out.emitInstruction(Inst, getSTI());
    return false;
  case Match_MnemonicFail:
    return Error(IDLoc, "unrecognized instruction mnemonic");
  case Match_InvalidOperand: {
    SMLoc ErrorLoc = IDLoc;
    if (ErrorInfo != ~0ULL) {
      if (ErrorInfo >= Operands.size())
        return Error(IDLoc, "too few operands for instruction");
      ErrorLoc = ((ISA32_LMOperand &)*Operands[ErrorInfo]).getStartLoc();
      if (ErrorLoc == SMLoc())
        ErrorLoc = IDLoc;
    }
    return Error(ErrorLoc, "invalid operand for instruction");
  }
  default:
    return Error(IDLoc, "unknown match failure");
  }
}

ParseStatus ISA32_LMAsmParser::tryParseRegister(MCRegister &Reg,
                                                SMLoc &StartLoc,
                                                SMLoc &EndLoc) {
  StartLoc = getLexer().getLoc();
  EndLoc = getLexer().getTok().getEndLoc();

  if (getLexer().getKind() != AsmToken::Identifier)
    return ParseStatus::NoMatch;

  StringRef Name = getLexer().getTok().getString();
  Reg = MatchRegisterName(Name);

  if (!Reg) {
    std::string UpperName = Name.upper();
    Reg = MatchRegisterName(UpperName);
  }

  if (!Reg)
    return ParseStatus::NoMatch;

  getLexer().Lex(); // Eat identifier token
  return ParseStatus::Success;
}

bool ISA32_LMAsmParser::parseRegister(MCRegister &Reg, SMLoc &StartLoc,
                                      SMLoc &EndLoc) {
  ParseStatus Res = tryParseRegister(Reg, StartLoc, EndLoc);
  if (!Res.isSuccess())
    return Error(StartLoc, "expected register name");
  return false;
}

ParseStatus ISA32_LMAsmParser::parseConditionCode(OperandVector &Operands) {
  SMLoc S = getLexer().getLoc();
  SMLoc E = getLexer().getTok().getEndLoc();

  if (getLexer().getKind() == AsmToken::Identifier) {
    StringRef CondStr = getLexer().getTok().getString();
    unsigned CondVal = StringSwitch<unsigned>(CondStr.upper())
                           .Case("EQ", 0)
                           .Case("NE", 1)
                           .Case("N", 2)
                           .Case("NN", 3)
                           .Case("C", 4)
                           .Case("NC", 5)
                           .Case("OV", 6)
                           .Case("NOV", 7)
                           .Default(~0U);
    if (CondVal != ~0U) {
      getLexer().Lex();
      const MCExpr *Expr = MCConstantExpr::create(CondVal, getContext());
      Operands.push_back(ISA32_LMOperand::CreateImm(Expr, S, E));
      return ParseStatus::Success;
    }
  }

  // Fallback to parsing expression for numerical condition
  const MCExpr *Expr = nullptr;
  if (!getParser().parseExpression(Expr, E)) {
    Operands.push_back(ISA32_LMOperand::CreateImm(Expr, S, E));
    return ParseStatus::Success;
  }

  return ParseStatus::NoMatch;
}

bool ISA32_LMAsmParser::parseOperand(OperandVector &Operands,
                                     StringRef Mnemonic) {
  SMLoc S = getLexer().getLoc();
  SMLoc E = getLexer().getTok().getEndLoc();

  // Try parsing register first
  MCRegister Reg;
  ParseStatus RegRes = tryParseRegister(Reg, S, E);
  if (RegRes.isSuccess()) {
    Operands.push_back(ISA32_LMOperand::CreateReg(Reg, S, E));
    return false;
  }

  // Check for condition code operand in BRH
  if (Mnemonic.equals_insensitive("BRH") && Operands.size() == 1) {
    ParseStatus CondRes = parseConditionCode(Operands);
    if (CondRes.isSuccess())
      return false;
  }

  // Parse expression / immediate / symbol
  const MCExpr *Expr = nullptr;
  if (!getParser().parseExpression(Expr, E)) {
    Operands.push_back(ISA32_LMOperand::CreateImm(Expr, S, E));
    return false;
  }

  return Error(S, "cannot parse operand");
}

bool ISA32_LMAsmParser::parseInstruction(ParseInstructionInfo &Info,
                                         StringRef Name, SMLoc NameLoc,
                                         OperandVector &Operands) {
  // Push primary mnemonic token
  Operands.push_back(ISA32_LMOperand::CreateToken(Name, NameLoc));

  // Check for multi-word mnemonics like "SLT ADD", "CHAR LOD", "H LDI", etc.
  if (Name.equals_insensitive("SLT") || Name.equals_insensitive("CHAR") ||
      Name.equals_insensitive("SHORT") || Name.equals_insensitive("INT") ||
      Name.equals_insensitive("H")) {
    if (getLexer().is(AsmToken::Identifier)) {
      StringRef SubMnemonic = getLexer().getTok().getString();
      SMLoc SubLoc = getLexer().getLoc();
      Operands.push_back(ISA32_LMOperand::CreateToken(SubMnemonic, SubLoc));
      getLexer().Lex();
    }
  }

  // Parse operands separated by commas
  if (getLexer().isNot(AsmToken::EndOfStatement)) {
    while (true) {
      if (parseOperand(Operands, Name))
        return true;

      if (getLexer().is(AsmToken::Comma)) {
        getLexer().Lex(); // consume comma
      } else if (getLexer().is(AsmToken::EndOfStatement)) {
        break;
      } else {
        return Error(getLexer().getLoc(), "unexpected token in operand list");
      }
    }
  }

  getLexer().Lex(); // consume EndOfStatement
  return false;
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeISA32_LMAsmParser() {
  RegisterMCAsmParser<ISA32_LMAsmParser> X(getTheISA32_LMTarget());
}
