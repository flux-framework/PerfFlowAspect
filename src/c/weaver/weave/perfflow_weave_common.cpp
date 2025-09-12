/************************************************************\
 * Copyright 2021 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#include "perfflow_weave_common.hpp"

using namespace llvm;

/******************************************************************************
 *                                                                            *
 *                 Public Method of WeaveCommon Class                         *
 *                                                                            *
 ******************************************************************************/

bool weave_ns::WeaveCommon::modifyAnnotatedFunctions(Module &m)
{
    auto annotations = m.getNamedGlobal("llvm.global.annotations");
    if (!annotations)
    {
        return false;
    }

    // Support Caliper annotations by greating the cali_begin_region
    // and cali_end_region functions.
#ifdef PERFFLOWASPECT_WITH_CALIPER
    IRBuilder<> IRB(m.getContext());
    AttrBuilder AB(m.getContext());
    AB.addAttribute(Attribute::AlwaysInline);
    //It was not added in LLVM@10.
    //AB.addAttribute(Attribute::ArgMemOnly);
    AttributeList Attrs = AttributeList::get(m.getContext(),
                          AttributeList::FunctionIndex, AB);
    // Insert Functions on the module
    CaliBeginRegion = m.getOrInsertFunction("cali_begin_region", Attrs,
                                            IRB.getVoidTy(), IRB.getPtrTy());
    CaliEndRegion = m.getOrInsertFunction("cali_end_region", Attrs, IRB.getVoidTy(),
                                          IRB.getPtrTy());
#endif


    bool changed = false;

    if (annotations->getNumOperands() <= 0)
    {
        return changed;
    }

    auto a = cast<ConstantArray> (annotations->getOperand(0));
    for (unsigned int i = 0; i < a->getNumOperands(); i++)
    {
        auto e = cast<ConstantStruct> (a->getOperand(i));
        auto *fn = dyn_cast<Function> (e->getOperand(0));
        if (fn != NULL)
        {
#ifdef PERFFLOWASPECT_WITH_CALIPER
            // We insert Caliper Instrumentation before weaver.
            // Thus weaver will include Caliper overheads
            changed |= instrumentCaliper(m, *fn);
#endif

            auto anno = cast<ConstantDataArray>(
                            cast<GlobalVariable>(e->getOperand(1))
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

/******************************************************************************
 *                                                                            *
 *                 Private Methods of WeaveCommon Class                         *
 *                                                                            *
 ******************************************************************************/

bool weave_ns::WeaveCommon::insertAfter(Module &m, Function &f, StringRef &a,
                                        int async, std::string &scope, std::string &flow, std::string pcut)
{
    if (m.empty() || f.empty())
    {
        return false;
    }

    auto &context = m.getContext();
    Type *voidType = Type::getVoidTy(context);
    Type *int32Type = Type::getInt32Ty(context);
    Type *int8Type = Type::getInt8Ty(context);
    std::vector<llvm::Type *> params;
    params.push_back(int32Type);
    params.push_back(int8Type);
    params.push_back(int8Type);
    params.push_back(int8Type);
    params.push_back(int8Type);
    params.push_back(int8Type);

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
        bool valid = false;
        if (isa<ReturnInst>(inst) || isa<ResumeInst>(inst))
        {
            valid = true;
        } else if (isa<UnreachableInst>(inst)) {
            if (auto *call = dyn_cast<CallInst>(inst->getPrevNode())) {
                Function *callee = call->getCalledFunction();
                if (callee && callee->getName() == "pthread_exit" || callee->getName() == "exit" || callee->getName() == "abort") {
                    inst = inst->getPrevNode();
                    valid = true;
                }
            }
        }

        if (valid) {
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

bool weave_ns::WeaveCommon::insertBefore(Module &m, Function &f, StringRef &a,
        int async, std::string &scope, std::string &flow, std::string pcut)
{
    if (m.empty() || f.empty())
    {
        return false;
    }

    auto &context = m.getContext();
    Type *voidType = Type::getVoidTy(context);
    Type *int32Type = Type::getInt32Ty(context);
    Type *int8Type = Type::getInt8Ty(context);
    std::vector<llvm::Type *> params;
    params.push_back(int32Type);
    params.push_back(int8Type);
    params.push_back(int8Type);
    params.push_back(int8Type);
    params.push_back(int8Type);
    params.push_back(int8Type);

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

#ifdef PERFFLOWASPECT_WITH_CALIPER
bool weave_ns::WeaveCommon::instrumentCaliper(Module &M, Function &F)
{
    IRBuilder<> IRB(M.getContext());
    BasicBlock &Entry = F.getEntryBlock();
    SplitBlock(&Entry, &*Entry.getFirstInsertionPt());

    IRB.SetInsertPoint(Entry.getTerminator());
    std::string FunctionName = F.getName().str();
    auto *FnStr = IRB.CreateGlobalStringPtr(FunctionName);
    IRB.CreateCall(CaliBeginRegion, {FnStr});

    bool RetFound = false;
    for (inst_iterator It = inst_begin(F), E = inst_end(F); It != E; ++It)
    {
        Instruction *I = &*It;
        if (!isa<ReturnInst>(I) && !isa<CallInst>(I) && !isa<InvokeInst>(I))
        {
            continue;
        }

        if (isa<ReturnInst>(I))
        {
            IRB.SetInsertPoint(I);
            IRB.CreateCall(CaliEndRegion, {FnStr});
            RetFound = true;
        }

        // This is a call instruction
        CallBase *CB = dyn_cast<CallBase>(I);
        if (CB && CB->doesNotReturn())
        {
            IRB.SetInsertPoint(I);
            IRB.CreateCall(CaliEndRegion, {FnStr});
            RetFound = true;
        }
    }

    // All functions need to have at least one exit block
    if (!RetFound)
    {
        dbgs() << "Could not find return for " << FunctionName << "\n";
        abort();
    }

    return RetFound;
}
#endif

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
