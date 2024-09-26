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
    outs() << "I am in modifyAnnotatedFunctions \n";

    auto annotations = m.getNamedGlobal("llvm.global.annotations");
    if (!annotations)
    {
        return false;
    }

    bool changed = false;

    if (annotations->getNumOperands() <= 0)
    {
        outs() << "I have failed as there are no operands!\n";
        return changed;
    }
    else
    {
        outs() << "Annotations has " << annotations->getNumOperands() << " operands.\n";
        outs() << "Annotations has " << annotations->getOperand(0)->getName() << " name for operand 0.\n";
    }

    auto a = cast<ConstantArray> (annotations->getOperand(0));
    for (unsigned int i = 0; i < a->getNumOperands(); i++)
    {
        auto e = cast<ConstantStruct> (a->getOperand(i));
        if (!e)
        {
            outs() << "I failed here at obtaining the struct. \n";
        }
        else
        {
            outs() << "e has " << e->getNumOperands() << " operands.\n";
            outs() << "Printing e's operands: \n";
            for (unsigned int k=0; k < e->getNumOperands(); k++)
            {   
                outs() << " Operand " << k << " is " << e->getOperand(k)->getName();
                outs() << "Operand " << k << "has " << e->getOperand(k)->getNumOperands() << "operands \n";    
            }
            // outs() << "e's 0 operand has " << e->getOperand(0)->getNumOperands() << "\n";
            // outs() << "e's 1 operand has " << e->getOperand(1)->getNumOperands() << "\n";
            // outs() << "e's 2 operand has " << e->getOperand(2)->getNumOperands() << "\n";
            // outs() << "e's 3 operand has " << e->getOperand(3)->getNumOperands() << "\n";
            // outs() << "e's 4 operand has " << e->getOperand(4)->getNumOperands() << "\n";
            // outs() << "e's 0 operand name " << e->getOperand(0)->getName() << "\n";
            // outs() << "e's 1 operand name " << e->getOperand(1)->getName() << "\n";
            // outs() << "e's 2 operand name " << e->getOperand(2)->getName() << "\n";
            // outs() << "e's 3 operand name " << e->getOperand(3)->getName() << "\n";
            // outs() << "e's 4 operand name " << e->getOperand(4)->getName() << "\n";
        }

        if (auto *fn = dyn_cast<Function> (e->getOperand(0)->getOperand(0)))
        {
            outs() << "I entered the part where we parse annotations. \n";
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

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
