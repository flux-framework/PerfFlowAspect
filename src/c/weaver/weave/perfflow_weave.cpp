/************************************************************\
 * Copyright 2021 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#include "llvm/IR/Value.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/LegacyPassManager.h"
#ifdef PERFFLOWASPECT_CLANG_11_NEWER
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
#include "perfflow_weave.hpp"

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <adiak_tool.h>
#define UNUSED(x) (void)(x)

using namespace llvm;


/******************************************************************************
 *                                                                            *
 *                 Private Methods of WeavingPass Class                       *
 *                                                                            *
 ******************************************************************************/

bool WeavingPass::insertAfter(Module &m, Function &f, StringRef &a,
                              int async, std::string &scope, std::string &flow, std::string pcut)
{
    if (m.empty() || f.empty())
    {
        return false;
    }

    auto &context = m.getContext();
    Type *voidType = Type::getVoidTy(context);
    Type *int32Type = Type::getInt32Ty(context);
    Type *int8PtrType = Type::getInt8PtrTy(context);
    std::vector<llvm::Type *> params;
    params.push_back(int32Type);
    params.push_back(int8PtrType);
    params.push_back(int8PtrType);
    params.push_back(int8PtrType);
    params.push_back(int8PtrType);
    params.push_back(int8PtrType);

    // voidType is return type, params are parameters and no variable length args
    FunctionType *weaveFuncTy = FunctionType::get(voidType, params, false);
    // Note: Use FunctionCallee after for Clang 9 or higher
    FunctionCallee after = m.getOrInsertFunction("perfflow_weave_after",
                           weaveFuncTy);
    // Constant *after = m.getOrInsertFunction("perfflow_weave_after",
    //                                        weaveFuncTy);

    // iterate through blocks
    for (BasicBlock &bb : f)
    {
        Instruction *inst = bb.getTerminator();
        if (isa<ReturnInst>(inst) || isa<ResumeInst>(inst))
        {
            IRBuilder<> builder(inst);
            Value *v1 = builder.CreateGlobalStringPtr(m.getName(), "str");
            Value *v2 = builder.CreateGlobalStringPtr(f.getName(), "str");
            Value *v3 = builder.CreateGlobalStringPtr(StringRef(scope), "str");
            Value *v4 = builder.CreateGlobalStringPtr(StringRef(flow), "str");
            Value *v5 = builder.CreateGlobalStringPtr(StringRef(pcut), "str");
            std::vector<Value *> args;
            args.push_back(ConstantInt::get(Type::getInt32Ty(context), async));
            args.push_back(v1);
            args.push_back(v2);
            args.push_back(v3);
            args.push_back(v4);
            args.push_back(v5);
            builder.CreateCall(after, args);
        }
    }
    return true;
}

bool WeavingPass::insertBefore(Module &m, Function &f, StringRef &a,
                               int async, std::string &scope, std::string &flow, std::string pcut)
{
    if (m.empty() || f.empty())
    {
        return false;
    }

    auto &context = m.getContext();
    Type *voidType = Type::getVoidTy(context);
    Type *int32Type = Type::getInt32Ty(context);
    Type *int8PtrType = Type::getInt8PtrTy(context);
    std::vector<llvm::Type *> params;
    params.push_back(int32Type);
    params.push_back(int8PtrType);
    params.push_back(int8PtrType);
    params.push_back(int8PtrType);
    params.push_back(int8PtrType);
    params.push_back(int8PtrType);

    // voidType is return type, params are parameters and no variable length args
    FunctionType *weaveFuncTy = FunctionType::get(voidType, params, false);
    // Note: User FunctionCallee before for Clang >= 9.0
    FunctionCallee before = m.getOrInsertFunction("perfflow_weave_before",
                            weaveFuncTy);
    //Constant *before = m.getOrInsertFunction ("perfflow_weave_before",
    //                                          weaveFuncTy);
    auto &entry = f.getEntryBlock();
    IRBuilder<> builder(&entry);
    Value *v1 = builder.CreateGlobalStringPtr(m.getName(), "str");
    Value *v2 = builder.CreateGlobalStringPtr(f.getName(), "str");
    Value *v3 = builder.CreateGlobalStringPtr(StringRef(scope), "str");
    Value *v4 = builder.CreateGlobalStringPtr(StringRef(flow), "str");
    Value *v5 = builder.CreateGlobalStringPtr(StringRef(pcut), "str");
    builder.SetInsertPoint(&entry, entry.begin());
    std::vector<Value *> args;
    args.push_back(ConstantInt::get(Type::getInt32Ty(context), async));
    args.push_back(v1);
    args.push_back(v2);
    args.push_back(v3);
    args.push_back(v4);
    args.push_back(v5);
    builder.CreateCall(before, args);

    return true;
}


/******************************************************************************
 *                                                                            *
 *                 Public Methods of WeavingPass Class                        *
 *                                                                            *
 ******************************************************************************/

// static void print_value(adiak_value_t *val, adiak_datatype_t *t)
// {
//    if (!t)
//       printf("ERROR");
//    switch (t->dtype) {
//       case adiak_type_unset:
//          printf("UNSET");
//          break;
//       case adiak_long:
//          printf("%ld", val->v_long);
//          break;
//       case adiak_ulong:
//          printf("%lu", (unsigned long) val->v_long);
//          break;
//       case adiak_longlong:
//          printf("%lld", val->v_longlong);
//          break;
//       case adiak_ulonglong:
//          printf("%llu", (unsigned long long) val->v_longlong);
//          break;
//       case adiak_int:
//          printf("%d", val->v_int);
//          break;
//       case adiak_uint:
//          printf("%u", (unsigned int) val->v_int);
//          break;
//       case adiak_double:
//          printf("%f", val->v_double);
//          break;
//       case adiak_date: {
//          char datestr[512];
//          signed long seconds_since_epoch = (signed long) val->v_long;
//          struct tm *loc = localtime(&seconds_since_epoch);
//          strftime(datestr, sizeof(datestr), "%a, %d %b %Y %T %z", loc);
//          printf("%s", datestr);
//          break;
//       }
//       case adiak_timeval: {
//          struct timeval *tval = (struct timeval *) val->v_ptr;
//          double duration = tval->tv_sec + (tval->tv_usec / 1000000.0);
//          printf("%fs (timeval)", duration);
//          break;
//       }
//       case adiak_version: {
//          char *s = (char *) val->v_ptr;
//          printf("%s (version)", s);
//          break;
//       }
//       case adiak_string: {
//          char *s = (char *) val->v_ptr;
//          printf("%s (string)", s);
//          break;
//       }
//       case adiak_catstring: {
//          char *s = (char *) val->v_ptr;
//          printf("%s (catstring)", s);
//          break;
//       }
//       case adiak_path: {
//          char *s = (char *) val->v_ptr;
//          printf("%s (path)", s);
//          break;
//       }
//       case adiak_range: {
//          adiak_value_t subvals[2];
//          adiak_datatype_t* subtypes[2];

//          adiak_get_subval(t, val, 0, subtypes+0, subvals+0);
//          adiak_get_subval(t, val, 1, subtypes+1, subvals+1);

//          print_value(subvals+0, *(subtypes+0));
//          printf(" - ");
//          print_value(subvals+1, *(subtypes+1));
//          break;
//       }
//       case adiak_set: {
//          printf("[");
//          for (int i = 0; i < adiak_num_subvals(t); i++) {
//             adiak_value_t subval;
//             adiak_datatype_t* subtype;
//             adiak_get_subval(t, val, i, &subtype, &subval);
//             print_value(&subval, subtype);
//             if (i+1 != t->num_elements)
//                printf(", ");
//          }
//          printf("]");
//          break;
//       }
//       case adiak_list: {
//          printf("{");
//          for (int i = 0; i < adiak_num_subvals(t); i++) {
//             adiak_value_t subval;
//             adiak_datatype_t* subtype;
//             adiak_get_subval(t, val, i, &subtype, &subval);
//             print_value(&subval, subtype);
//             if (i+1 != t->num_elements)
//                printf(", ");
//          }
//          printf("}");
//          break;
//       }
//       case adiak_tuple: {
//          printf("(");
//          for (int i = 0; i < adiak_num_subvals(t); i++) {
//             adiak_value_t subval;
//             adiak_datatype_t* subtype;
//             adiak_get_subval(t, val, i, &subtype, &subval);
//             print_value(&subval, subtype);
//             if (i+1 != t->num_elements)
//                printf(", ");
//          }
//          printf(")");
//          break;
//       }
//    }
// }

static void print_nameval(const char *name, int category, const char *subcategory, adiak_value_t *value, adiak_datatype_t *t, void *opaque_value)
{
    // printf("%s: ", name);
    printf("%s: ", "In the callback print");
    outs() << "In the callback \n";
    // print_value(value, t);
    // printf("\n");
}

bool WeavingPass::doInitialization(Module &m)
{
    adiak_register_cb(1, adiak_category_all, print_nameval, 0, NULL);
    outs() << "WeavePass loaded successfully. \n";

    auto annotations = m.getNamedGlobal("llvm.global.annotations");
    if (!annotations)
    {
        return false;
    }

    bool changed = false;
    auto a = cast<ConstantArray> (annotations->getOperand(0));
    for (unsigned int i = 0; i < a->getNumOperands(); i++)
    {
        auto e = cast<ConstantStruct> (a->getOperand(i));
        if (auto *fn = dyn_cast<Function> (e->getOperand(0)->getOperand(0)))
        {
            auto anno = cast<ConstantDataArray>(
                            cast<GlobalVariable>(e->getOperand(1)->getOperand(0))
                            ->getOperand(0))
                        ->getAsCString();
            std::string pcut, scope, flow;
            if (perfflow_parser_parse(anno.data(), pcut, scope, flow) == 0)
            {
                if (pcut == "around" || pcut == "before")
                    changed = insertBefore(m, *fn,
                                           anno, 0, scope, flow, pcut) || changed;
                else if (pcut == "around_async" || pcut == "before_async")
                {
                    changed = insertBefore(m, *fn,
                                           anno, 1, scope, flow, pcut) || changed;
                }
                if (pcut == "around" || pcut == "after")
                {
                    if (pcut == "around")
                    {
                        if (flow == "in" || flow == "out")
                        {
                            flow = "NA";
                        }
                    }
                    changed = insertAfter(m, *fn,
                                          anno, 0, scope, flow, pcut) || changed;
                }
                else if (pcut == "around_async" || pcut == "after_async")
                {
                    if (pcut == "around")
                    {
                        if (flow == "in" || flow == "out")
                        {
                            flow = "NA";
                        }
                    }
                    changed = insertAfter(m, *fn,
                                          anno, 1, scope, flow, pcut) || changed;
                }
            }
            else
            {
                errs() << "WeavePass[WARN]: Ignoring " << anno << "\n";
            }
        }
    }
    return changed;
}

bool WeavingPass::runOnFunction(Function &F)
{
    return false;
}

char WeavingPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerWeavingPass(const PassManagerBuilder &,
                                legacy::PassManagerBase &PM)
{
    PM.add(new WeavingPass());
}

static RegisterStandardPasses
RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
               registerWeavingPass);

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
