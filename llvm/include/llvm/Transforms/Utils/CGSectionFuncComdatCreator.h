//===-- CGSectionFuncComdatCreator.h - CG func comdat creator ---*- C++ -*-===//
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

#ifndef LLVM_TRANSFORMS_UTILS_CGSECTIONFUNCCOMDATCREATOR_H
#define LLVM_TRANSFORMS_UTILS_CGSECTIONFUNCCOMDATCREATOR_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class CGSectionFuncComdatCreatorPass
    : public PassInfoMixin<CGSectionFuncComdatCreatorPass> {
public:
  PreservedAnalyses run(Module &, ModuleAnalysisManager &);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_UTILS_CGSECTIONFUNCCOMDATCREATOR_H

