/************************************************************\
 * Copyright 2021 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#ifndef PERFFLOW_WEAVE_COMMON_H
#define PERFFLOW_WEAVE_COMMON_H

#include "llvm/IR/Value.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/LegacyPassManager.h"
#if defined(PERFFLOWASPECT_CLANG_11_NEWER) || defined(PERFFLOWASPECT_CLANG_15_NEWER)
#include "llvm/IR/AbstractCallSite.h"
#else
#include "llvm/IR/CallSite.h"
#endif
#include "llvm/IR/Module.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Support/raw_ostream.h"

#include "../../parser/perfflow_parser.hpp"

using namespace llvm;

namespace weave_ns {
    
class WeaveCommon
{
private:
    bool insertAfter(Module &m, Function &f, StringRef &a,
                              int async, std::string &scope, std::string &flow, std::string pcut);


    bool insertBefore(Module &m, Function &f, StringRef &a,
                               int async, std::string &scope, std::string &flow, std::string pcut); 

public:
    bool modifyAnnotatedFunctions(Module &m);

}; 

} // End namespace

#endif // PERFFLOW_WEAVE_COMMON_H


/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
