add_llvm_component_library(LLVMISA32_LMDesc
  ISA32_LMInstPrinter.cpp
  ISA32_LMMCAsmInfo.cpp
  ISA32_LMMCTargetDesc.cpp

  LINK_COMPONENTS
  ISA32_LMInfo
  MC
  Support
  TargetParser

  ADD_TO_COMPONENT
  ISA32_LM
  )
