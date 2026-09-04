add_llvm_component_library(LLVMISA32_LMAsmParser
  ISA32_LMAsmParser.cpp

  LINK_COMPONENTS
  MC
  MCParser
  ISA32_LMDesc
  ISA32_LMInfo
  Support
  TargetParser

  ADD_TO_COMPONENT
  ISA32_LM
  )
