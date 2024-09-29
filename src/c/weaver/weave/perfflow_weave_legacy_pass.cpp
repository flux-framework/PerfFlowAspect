/************************************************************\
 * Copyright 2021 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Support/raw_ostream.h"

#include "perfflow_weave_legacy_pass.hpp"
#include "perfflow_weave_common.hpp"

using namespace llvm;

/******************************************************************************
 *                                                                            *
 *                 Public Methods of LegacyWeavingPass Class                        *
 *                                                                            *
 ******************************************************************************/

bool LegacyWeavingPass::doInitialization(Module &m)
{
    outs() << "WeavePass loaded successfully. \n";

    bool changed = false;

    WeaveCommon weaver;
    changed = weaver.modifyAnnotatedFunctions(m);

    return changed;
}

bool LegacyWeavingPass::runOnFunction(Function &F)
{
    return false;
}

char LegacyWeavingPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerWeavingPass(const PassManagerBuilder &,
                                legacy::PassManagerBase &PM)
{
    PM.add(new LegacyWeavingPass());
}

static RegisterStandardPasses
RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
               registerWeavingPass);

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
