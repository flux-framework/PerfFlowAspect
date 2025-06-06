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

    #ifdef PERFFLOWASPECT_WITH_ADIAK
    Function *main = m.getFunction("main");

    if (main != NULL) {
        outs() << "Inserting Adiak?\n";
        insertAdiak(m, *main);
    }
    else {
        outs () << "No main";
    }
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

#ifdef PERFFLOWASPECT_WITH_ADIAK
bool weave_ns::WeaveCommon::insertAdiak(Module &m, Function &f) {
    LLVMContext &context = m.getContext();

    // the adiak_init() call
    // return and argument types
    Type *voidTy = Type::getVoidTy(context);
    Type *int8Ty = Type::getInt8Ty(context);
    Type *int32Ty = Type::getInt32Ty(context);
    PointerType *voidPtrTy = PointerType::getUnqual(int8Ty);

    // adiak_init function signature
    std::vector<Type*> initArgs = { voidPtrTy };
    FunctionType *adiakInitType = FunctionType::get(voidTy, initArgs, false);
    FunctionCallee adiakInit = m.getOrInsertFunction("adiak_init", adiakInitType);

    IRBuilder<> builder(context);
    Value *arg;
    BasicBlock &entry = f.getEntryBlock();
    builder.SetInsertPoint(&entry, entry.begin());

    #ifdef PERFFLOWASPECT_WITH_MPI
    // find the MPI communicator, MPI_COMM_WORLD
    // OpenMPI exposes this as ompi_comm_world (untested)
    // MPICH exposes this as a constant 0x44000000
    if (GlobalVariable *gv = m.getGlobalVariable("ompi_comm_world")) {
        arg = builder.CreateLoad(gv->getValueType(), gv);
    }
    else {
        AllocaInst *alloc = builder.CreateAlloca(int32Ty, nullptr, "weave_mpi_comm");
        uint64_t mpiValue = 0x44000000;
        Value *commVal = ConstantInt::get(int32Ty, mpiValue);
        builder.CreateStore(commVal, alloc);
        arg = builder.CreateBitCast(
            alloc,
            voidPtrTy,
            "mpi_comm_world_void"
        );
    }

    CallInst *mpi = nullptr;
    // find each instruction to see if there is a call instruction
    for (BasicBlock &bb : f) {
        for (Instruction &i : bb) {
            if (auto *call = dyn_cast<CallInst>(&i)) {
                if (Function *callee = call->getCalledFunction()) {
                    if (callee->getName() == "MPI_Init") {
                        mpi = call;
                    }
                }
            }
        }
        if (mpi) {
            auto insertPosition = BasicBlock::iterator(mpi);
            ++insertPosition;
            builder.SetInsertPoint(mpi->getParent(), insertPosition);
            builder.CreateCall(adiakInit, {arg});
            break;

        }
    }

    if (!mpi) {
        arg = Constant::getNullValue(voidPtrTy);
        builder.CreateCall(adiakInit, {arg});
    }
    #else
    arg = Constant::getNullValue(voidPtrTy);
    builder.CreateCall(adiakInit, {arg});
    #endif

    // call adiak_collect_all()
    FunctionType *collectType = FunctionType::get(int32Ty, {}, false);
    FunctionCallee collectAll = m.getOrInsertFunction("adiak_collect_all", collectType);
    builder.CreateCall(collectAll, {});

    // adiak_fini signature
    FunctionType *adiakFinishType = FunctionType::get(voidTy, {}, false);
    FunctionCallee adiakFinish = m.getOrInsertFunction("adiak_fini", adiakFinishType);

    // find all places where the function terminates from a ReturnInst
    std::vector<ReturnInst*> returns;
    for (BasicBlock &bb : f) {
        if (auto *ret = dyn_cast<ReturnInst>(bb.getTerminator())) {
            returns.push_back(ret);
        }
    }

    // insert adiak_fini at those return instructions
    for (ReturnInst *ret : returns) {
        builder.SetInsertPoint(ret);
        builder.CreateCall(adiakFinish, {});
    }

    return true;
}
#endif

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
