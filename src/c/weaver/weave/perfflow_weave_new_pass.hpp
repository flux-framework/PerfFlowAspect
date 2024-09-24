/************************************************************\
 * Copyright 2021 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#ifndef PERFFLOW_WEAVE_NEW_PASS_H
#define PERFFLOW_WEAVE_NEW_PASS_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

#include <string>

using namespace llvm;

namespace {

class NewWeavingPass : public PassInfoMixin<NewWeavingPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &);
  bool runOnModule(Module &M);

  // Without isRequired returning true, this pass will be skipped for functions
  // decorated with the optnone LLVM attribute. Note that clang -O0 decorates
  // all functions with optnone.
  static bool isRequired() { return true; }

};

}

#endif // PERFFLOW_WEAVE_NEW_PASS_H


/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
