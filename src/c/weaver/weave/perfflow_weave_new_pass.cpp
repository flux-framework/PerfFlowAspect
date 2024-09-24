/************************************************************\
 * Copyright 2021 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Support/raw_ostream.h"

#include "perfflow_weave_new_pass.hpp"
#include "perfflow_weave_common.hpp"
#include <iostream>
#include <cstring>

using namespace llvm;

/******************************************************************************
 *                                                                            *
 *                 Public Methods of NewWeavingPass Class                         *
 *                                                                            *
 ******************************************************************************/

// Implement the runOnModule Function
bool NewWeavingPass::runOnModule(Module &M)
{
    bool changed = false;
    outs() << "NewWeavePass loaded successfully! \n";

    WeaveCommon weaver;
    changed = weaver.modifyAnnotatedFunctions(M);
    
    return changed;
}

// Run on module function.
PreservedAnalyses NewWeavingPass::run(Module &M,
                                      ModuleAnalysisManager &)
{
    bool changed = runOnModule(M);

    return (changed ? llvm::PreservedAnalyses::none()
            : llvm::PreservedAnalyses::all());
}

// Register the new pass.
// See docs here: https://github.com/banach-space/llvm-tutor/blob/main/lib/InjectFuncCall.cpp#L139
// And here: https://rocmdocs.amd.com/projects/llvm-project/en/latest/LLVM/llvm/html/WritingAnLLVMNewPMPass.html
// https://stackoverflow.com/questions/54447985/how-to-automatically-register-and-load-modern-pass-in-clang/75999804#75999804
// https://github.com/llvm/llvm-project/issues/56137

PassPluginLibraryInfo getNewWeavingPassPluginInfo()
{

    const auto pass_callback = [](PassBuilder & PB)
    {
        PB.registerPipelineEarlySimplificationEPCallback(
            [&](ModulePassManager & MPM, auto)
        {
            MPM.addPass(NewWeavingPass());
            return true;
        }
        );
    };

    return {LLVM_PLUGIN_API_VERSION, "new-weaving-pass", LLVM_VERSION_STRING, pass_callback};
}

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo()
{
    return getNewWeavingPassPluginInfo();
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
