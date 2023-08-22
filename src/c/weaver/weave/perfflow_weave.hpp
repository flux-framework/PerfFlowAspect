/************************************************************\
 * Copyright 2021 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#ifndef PERFFLOW_WEAVE_H
#define PERFFLOW_WEAVE_H

#include "llvm/IR/Attributes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <string>

using namespace llvm;

namespace {

class WeavingPass : public FunctionPass
{
public:
    FunctionCallee CaliBeginRegion;
    FunctionCallee CaliEndRegion;

    static char ID;
    WeavingPass () : FunctionPass (ID) {}
    virtual bool doInitialization (Module &m);
    virtual bool runOnFunction (Function &F);

private:
    bool insertAfter (Module &m, Function &f, StringRef &a,
                      int async, std::string &scope, std::string &flow, std::string pcut);
    bool insertBefore (Module &m, Function &f, StringRef &a,
                       int async, std::string &scope, std::string &flow, std::string pcut);

    bool instrumentCaliper(Module &M, Function &F);
};

}

#endif // PERFFLOW_WEAVE_H


/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
