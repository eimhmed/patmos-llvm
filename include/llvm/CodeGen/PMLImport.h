//==- PMLImport.h - Import PML information --------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass is used to import PML infos and provide them to LLVM passes.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CODEGEN_PMLIMPORT_H
#define LLVM_CODEGEN_PMLIMPORT_H

#include "llvm/Pass.h"
#include "llvm/CodeGen/PML.h"


namespace llvm {

  class MachineBasicBlock;
  class MachineFunction;
  class Module;

  class PMLImport : public ImmutablePass {
  private:
    virtual void anchor();

    std::vector<yaml::PMLDoc> YDocs;

    bool Initialized;

  public:
    static char ID;

    PMLImport()
    : ImmutablePass(ID), Initialized(false)
    {
      PassRegistry &Registry = *PassRegistry::getPassRegistry();
      initializePMLImportPass(Registry);
    }

    void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
    }

    // TODO Maybe at some later point, we might want to load multiple PML files
    // for different compiler phases, or a single PML file that contains
    // multiple results that are back-annotated to different compiler phases.
    // We will need some sort of interface to select the phase that we are
    // currently in (no need to be able to handle multiple phases in parallel,
    // as long as LLVM is not multithreaded), and need to export PML files at
    // different phases.

    virtual void initializePass();


    /// Check if any PML infos are actually available.
    bool isAvailable() const;

    /// TODO define a sane set of 'isAvailable' functions (?)
    bool isAvailable(const MachineFunction *MF) const;


    /// TODO define various getters to access infos including context, ..

    uint64_t getLocalWCET(const MachineBasicBlock *MBB) const;

    double getCriticality(const MachineBasicBlock *MBB) const;

  };

}

#endif
