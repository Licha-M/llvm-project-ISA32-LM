add_llvm_component_library(LLVMISA32_LMInfo
  ISA32_LMTargetInfo.cpp

  LINK_COMPONENTS
  MC
  Support

  ADD_TO_COMPONENT
  ISA32_LM
  )
