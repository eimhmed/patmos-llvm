//===-- MachineFunctionAnalysis.cpp ---------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the definitions of the MachineFunctionAnalysis members.
//
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/MachineFunctionAnalysis.h"
#include "llvm/CodeGen/GCMetadata.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

INITIALIZE_PASS(MachineFunctionAnalysis, "machinefunctionanalysis",
                "Machine Function Analysis", false, true)

char MachineFunctionAnalysis::ID = 0;

MachineFunctionAnalysis::MachineFunctionAnalysis() :
  FunctionPass(ID), TM(0), MF(0), PreserveMF(false),
  NextFnNum(0)
{
  initializeMachineFunctionAnalysisPass(*PassRegistry::getPassRegistry());
}

MachineFunctionAnalysis::MachineFunctionAnalysis(const TargetMachine &tm) :
  FunctionPass(ID), TM(&tm), MF(0), PreserveMF(false),
  NextFnNum(0)
{
  initializeMachineFunctionAnalysisPass(*PassRegistry::getPassRegistry());
}

MachineFunctionAnalysis::~MachineFunctionAnalysis() {
  releaseMemory();
  assert(!MF && "MachineFunctionAnalysis left initialized!");
}

void MachineFunctionAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<MachineModuleInfo>();
}

bool MachineFunctionAnalysis::doInitialization(Module &M) {
  // TODO with LLVM 3.3, MMI should have a doInitialization(Module) function,
  // then the MMI->setModule() call should be done there.
  MachineModuleInfo *MMI = getAnalysisIfAvailable<MachineModuleInfo>();
  assert(MMI && "MMI not around yet??");
  MMI->setModule(&M);
  NextFnNum = 0;
  return false;
}


bool MachineFunctionAnalysis::runOnFunction(Function &F) {
  assert(!MF && "MachineFunctionAnalysis already initialized!");

  // check whether a MachineFunction exists for F.
  MachineModuleInfo &MMI = getAnalysis<MachineModuleInfo>();
  MF = MMI.getMachineFunction(&F);

  // If function already exists and we want to restart, force creation
  if (MF && TM) {
    MMI.removeMachineFunction(&F);
    delete MF;
    MF = 0;
  }

  // no MachineFunction available? create one ...
  if (!MF && TM) {
    MF = new MachineFunction(&F, *TM, NextFnNum++, MMI,
                             getAnalysisIfAvailable<GCModuleInfo>());
  }
  else if (!TM) {
    // Note: We could retrieve the TargetMachine from MMI and pass a flag to the
    // constructor to force (re-)creation of MFs (this must be set in
    // LLVMTargetMachine, otherwise JIT recompilation breaks).
    // However, we need to make sure functions are numbered properly in that
    // case, so we need to persist the counter as well or use the number
    // of persisted functions to calculate the function-number.
    llvm_unreachable(
       "Creating a MachineFunction after a MachineModulePass is not supported");
  }

  return false;
}

void MachineFunctionAnalysis::releaseMemory() {
  // check whether a MachineFunction exists for F.
  MachineModuleInfo *MMI = getAnalysisIfAvailable<MachineModuleInfo>();

  if (PreserveMF && MMI && MF) {
    // store the MachineFunction instead of destroying it,
    // but only if MachineFunctionAnalysis has been initialized before
    // It seems that this happens sometimes when this analysis is free'd
    // but not used before
    MMI->putMachineFunction(MF, MF->getFunction());
  }
  else if (MMI && MF) {
    // If we have a MachineModuleInfo, cleanup there as well
    MMI->removeMachineFunction(MF->getFunction());
    delete MF;
  }
  else if (MF) {
    // No MachineModuleInfo, no need to remove anything
    delete MF;
  }
  MF = 0;
}
