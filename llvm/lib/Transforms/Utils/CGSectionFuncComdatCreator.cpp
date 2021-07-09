//===-- CGSectionFuncComdatCreator.cpp - CG section func comdat creator ---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass creates comdats for functions whose symbols will be referenced
// from the call graph section. These comdats are used to create the call graph
// sections, so that, the sections can get discarded by the linker if the
// functions get removed.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/CGSectionFuncComdatCreator.h"

#include "llvm/ADT/Triple.h"
#include "llvm/IR/Instructions.h"

namespace llvm {

namespace {
class ModuleCGSectionFuncComdatCreator {
public:
  bool instrumentModule(Module &);

private:
  bool createFunctionComdat(Function &F, const Triple &T) const;
  bool hasIndirectCalls(const Function &F) const;
  bool isTargetToIndirectCalls(const Function &F) const;
};

bool ModuleCGSectionFuncComdatCreator::createFunctionComdat(
    Function &F, const Triple &T) const {
  if (auto *Comdat = F.getComdat())
    return false;
  assert(F.hasName());
  Module *M = F.getParent();

  // Make a new comdat for the function. Use the "no duplicates" selection kind
  // if the object file format supports it. For COFF we restrict it to non-weak
  // symbols.
  Comdat *C = M->getOrInsertComdat(F.getName());
  if (T.isOSBinFormatELF() || (T.isOSBinFormatCOFF() && !F.isWeakForLinker()))
    C->setSelectionKind(Comdat::NoDuplicates);
  F.setComdat(C);
  return true;
}

bool ModuleCGSectionFuncComdatCreator::hasIndirectCalls(
    const Function &F) const {
  for (const auto &I : F)
    if (const CallInst *CI = dyn_cast<CallInst>(&I))
      if (CI->isIndirectCall())
        return true;
  return false;
}

bool ModuleCGSectionFuncComdatCreator::isTargetToIndirectCalls(
    const Function &F) const {
  return !F.hasLocalLinkage() ||
         F.hasAddressTaken(nullptr,
                           /* IgnoreCallbackUses */ true,
                           /* IgnoreAssumeLikeCalls */ true,
                           /* IgnoreLLVMUsed */ false);
}

bool ModuleCGSectionFuncComdatCreator::instrumentModule(Module &M) {
  Triple TargetTriple = Triple(M.getTargetTriple());

  bool CreatedComdats = false;

  for (Function &F : M) {
    if (isTargetToIndirectCalls(F) || hasIndirectCalls(F)) {
      if (TargetTriple.supportsCOMDAT() && !F.isInterposable() &&
          !F.isDeclarationForLinker()) {
        CreatedComdats |= createFunctionComdat(F, TargetTriple);
      }
    }
  }

  return CreatedComdats;
}

} // namespace

PreservedAnalyses
CGSectionFuncComdatCreatorPass::run(Module &M, ModuleAnalysisManager &AM) {
  ModuleCGSectionFuncComdatCreator ModuleCG;
  if (ModuleCG.instrumentModule(M)) {
    return PreservedAnalyses::none();
  }
  return PreservedAnalyses::all();
}

} // namespace llvm

