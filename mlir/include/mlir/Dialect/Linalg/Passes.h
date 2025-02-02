//===- Passes.h - Linalg pass entry points ----------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This header file defines prototypes that expose pass constructors.
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_DIALECT_LINALG_PASSES_H_
#define MLIR_DIALECT_LINALG_PASSES_H_

#include "mlir/Pass/Pass.h"

namespace mlir {
std::unique_ptr<OperationPass<FuncOp>> createConvertElementwiseToLinalgPass();

std::unique_ptr<OperationPass<FuncOp>> createLinalgFoldUnitExtentDimsPass();

std::unique_ptr<Pass> createLinalgFusionOfTensorOpsPass();
std::unique_ptr<Pass> createFoldReshapeOpsByLinearizationPass();

std::unique_ptr<OperationPass<FuncOp>>
createLinalgTilingPass(ArrayRef<int64_t> tileSizes = {});

std::unique_ptr<OperationPass<FuncOp>>
createLinalgTilingToParallelLoopsPass(ArrayRef<int64_t> tileSizes = {});

std::unique_ptr<OperationPass<FuncOp>>
createLinalgTilingToTiledLoopPass(ArrayRef<int64_t> tileSizes = {});

std::unique_ptr<OperationPass<FuncOp>>
createLinalgPromotionPass(bool dynamicBuffers, bool useAlloca);
std::unique_ptr<OperationPass<FuncOp>> createLinalgPromotionPass();

std::unique_ptr<OperationPass<FuncOp>> createLinalgInlineScalarOperandsPass();

/// Create a pass to convert Linalg tiled loops to `scf.for` and `scf.parallel`
/// loops and memref.load/memref.store accesses.
std::unique_ptr<OperationPass<FuncOp>> createConvertLinalgTiledLoopsToSCFPass();

/// Create a pass to convert Linalg operations to scf.for loops and
/// memref.load/memref.store accesses.
std::unique_ptr<OperationPass<FuncOp>> createConvertLinalgToLoopsPass();

/// Create a pass to convert Linalg operations to scf.parallel loops and
/// memref.load/memref.store accesses.
std::unique_ptr<OperationPass<FuncOp>> createConvertLinalgToParallelLoopsPass();

/// Create a pass to convert Linalg operations to affine.for loops and
/// affine_load/affine_store accesses.
/// Placeholder for now, this is NYI.
std::unique_ptr<OperationPass<FuncOp>> createConvertLinalgToAffineLoopsPass();

/// Create a pass that bufferizes the body of a FuncOp and tries to reuse the
/// buffers for those arguments that:
///   a) have been annotated 'inplaceable' and
///   b) whose buffer uses would be free of memory hazards.
std::unique_ptr<Pass> createLinalgComprehensiveFuncBufferizePass();

/// Create a pass to convert Linalg operations which work on tensors to use
/// buffers instead.
std::unique_ptr<OperationPass<FuncOp>> createLinalgBufferizePass();

/// Create a pass to conver named Linalg operations to Linalg generic
/// operations.
std::unique_ptr<OperationPass<FuncOp>> createLinalgGeneralizationPass();

/// Create a pass to convert Linalg operations to equivalent operations that
/// work on primitive types, if possible.
std::unique_ptr<Pass> createLinalgDetensorizePass();

//===----------------------------------------------------------------------===//
// Registration
//===----------------------------------------------------------------------===//

/// Generate the code for registering passes.
#define GEN_PASS_REGISTRATION
#include "mlir/Dialect/Linalg/Passes.h.inc"

} // namespace mlir

#endif // MLIR_DIALECT_LINALG_PASSES_H_
