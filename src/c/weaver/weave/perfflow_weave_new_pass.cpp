/************************************************************\
 * Copyright 2021 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

// #include "llvm/IR/Value.h"
// #include "llvm/IR/Attributes.h"
// #include "llvm/IR/Constants.h"
// #include "llvm/IR/AbstractCallSite.h"
// #include "llvm/IR/Module.h"
// #include "llvm/IR/Argument.h"
// #include "llvm/IR/IRBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Support/raw_ostream.h"

// #include "../../parser/perfflow_parser.hpp"
#include "perfflow_weave_new_pass.hpp"
#include "perfflow_weave_common.hpp"
#include <iostream>
#include <cstring>

using namespace llvm;
// using namespace std;

// Implement the runOnModule Function
bool NewWeavingPass::runOnModule(Module &M)
{
    bool changed = false;
    // We do nothing right now, let's build to see if this goes through and loads correctly.
    // The build will probably fail as we are not parsing the annotations.

    outs() << "NewWeavePass loaded successfully! \n";

    // The following loops through each function. This is where we need to check annotation
    // (from doInitialization in the legacy pass and add the insertBefore/insertAfter)

    WeaveCommon weaver; 
    changed = weaver.modifyAnnotatedFunctions(M);

    for (auto &F : M)
    {
        if (F.isDeclaration())
             continue;

        if (F.getName().str()=="main")
        {
             outs() << "We found main! We will insert Adiak call here eventually.\n";
             continue;
        }

    //     // Get an IR builder. Sets the insertion point to the top of the function
    //    IRBuilder<> Builder(&*F.getEntryBlock().getFirstInsertionPt());

    //     // Inject a global variable that contains the function name
    //      auto FuncName = Builder.CreateGlobalStringPtr(F.getName());

      

    //     // Printf requires i8*, but PrintfFormatStrVar is an array: [n x i8]. Add
    //     // a cast: [n x i8] -> i8*
    //     llvm::Value *FormatStrPtr =
    //         Builder.CreatePointerCast(PrintfFormatStrVar, PrintfArgTy, "formatStr");

    //     // The following is visible only if you pass -debug on the command line
    //     // *and* you have an assert build.
    //     LLVM_DEBUG(dbgs() << " Injecting call to printf inside " << F.getName()
    //                       << "\n");

    //     // Finally, inject a call to printf
    //     Builder.CreateCall(
    //         Printf, {FormatStrPtr, FuncName, Builder.getInt32(F.arg_size())});

    //     InsertedAtLeastOnePrintf = true;
    }

    return changed;
}

// Run on module function.
PreservedAnalyses NewWeavingPass::run(llvm::Module &M,
                                      llvm::ModuleAnalysisManager &)
{
    bool Changed = runOnModule(M);

    return (Changed ? llvm::PreservedAnalyses::none()
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