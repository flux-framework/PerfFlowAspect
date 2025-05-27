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

    // the beginning of main
    BasicBlock &entry = f.getEntryBlock();
    IRBuilder<> builder(&entry, entry.begin());

    // adiak_init function signature
    std::vector<Type*> initArgs = { voidPtrTy };
    FunctionType *adiakInitType = FunctionType::get(voidTy, initArgs, false);
    FunctionCallee adiakInit = m.getOrInsertFunction("adiak_init", adiakInitType);
    
    // call adiak_init(NULL)
    Value *nullPtr = Constant::getNullValue(voidPtrTy);
    builder.CreateCall(adiakInit, {nullPtr});

    // call adiak_collect_all()
    std::vector<Type*> collectArgs = { };  // No arguments
    FunctionType *collectType = FunctionType::get(int32Ty, collectArgs, false);
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

    // insert the print callback
    // Function *cb = printAdiakCallback(m);

    // register cb with adiak_list_namevals
    // Type *cbTy = cb->getType();
    // PointerType *cbPtrTy = PointerType::getUnqual(cbTy);
    // std::vector<Type*> listNamevalsArgs = { 
    //     int32Ty,
    //     int32Ty,
    //     cbPtrTy,
    //     voidPtrTy
    // };  
    // FunctionType *listNamevalsType = FunctionType::get(voidTy, listNamevalsArgs, false);
    // FunctionCallee listNamevals = m.getOrInsertFunction("adiak_list_namevals", listNamevalsType);

    // call the cb function
    // Value *adiakVersion = builder.getInt32(1);
    // Value *categoryAll = builder.getInt32(1);
    // Value *cbPtr = builder.CreateBitCast(cb, cbPtrTy);
    // builder.CreateCall(listNamevals, {adiakVersion, categoryAll, cbPtr, nullPtr});

    return true;
}

Function* weave_ns::WeaveCommon::printAdiakCallback(Module &m) {
    LLVMContext &context = m.getContext();
    Type *voidTy = Type::getVoidTy(context);
    Type *int8Ty = Type::getInt8Ty(context);
    Type *int32Ty = Type::getInt32Ty(context);
    Type *int64Ty = Type::getInt64Ty(context);
    StructType *adiakValueTy = StructType::getTypeByName(context, "union.adiak_value_t");
    StructType *datatypeTy = StructType::getTypeByName(context, "struct.adiak_datatype_t");
    if (!adiakValueTy)
        adiakValueTy = StructType::create(context, "union.adiak_value_t");
    if (!datatypeTy)
        datatypeTy = StructType::create(context, "struct.adiak_datatype_t");

    datatypeTy->setBody({
        int32Ty,
    }, false);
    PointerType *voidPtrTy = PointerType::getUnqual(int8Ty);
    PointerType *charPtrTy = PointerType::getUnqual(int8Ty);
    PointerType *valuePtrTy = PointerType::getUnqual(adiakValueTy);
    PointerType *datatypePtrTy = PointerType::getUnqual(datatypeTy);


    // void (*adiak_nameval_cb_t)(const char *name, 
    //      int category, const char *subcategory, 
    //          adiak_value_t *value, adiak_datatype_t *t, void *opaque_value)
    std::vector<llvm::Type*> cbArgs = {
        charPtrTy,           // const char *name
        int32Ty,             // adiak_category_t category
        charPtrTy,           // const char *subcategory
        valuePtrTy,          // adiak_value_t *value
        datatypePtrTy,       // adiak_datatype_t *t
        voidPtrTy            // void *opaque_value
    };

    // callback signature
    FunctionType *cbType = llvm::FunctionType::get(voidTy, cbArgs, false);
    Function *cb = Function::Create(cbType, Function::ExternalLinkage, "print_callback", &m);


    // parameter names
    auto argIter = cb->arg_begin();
    Value *nameParam = argIter++;
    nameParam->setName("name");
    Value *categoryParam = argIter++;
    categoryParam->setName("category");
    Value *subParam = argIter++;
    subParam->setName("subcategory");
    Value *valueParam = argIter++;
    valueParam->setName("value");
    Value *typeParam = argIter++;
    typeParam->setName("type");
    Value *argParam = argIter;
    argParam->setName("opaque");

    // function body
    BasicBlock *entry = BasicBlock::Create(context, "entry", cb);
    BasicBlock *defaultBlock = BasicBlock::Create(context, "default_case", cb);
    BasicBlock *uintCase = BasicBlock::Create(context, "uint_case", cb);
    BasicBlock *stringCase = BasicBlock::Create(context, "string_case", cb);
    BasicBlock *dateCase = BasicBlock::Create(context, "version_case", cb);


    IRBuilder<> builder(entry);

    // adiak_datatype_t struct. we only care about the first value, which contains the type
    Value *dtypePtr = builder.CreateStructGEP(
        datatypeTy,
        typeParam,
        0,
        "dtypePtr"
    );

    Value *dtypeVal = builder.CreateLoad(int32Ty, dtypePtr, "dtype");

    // find the printf functoin
    std::vector<llvm::Type*> printfArgs = { charPtrTy };
    FunctionType *printfType = llvm::FunctionType::get(int32Ty, printfArgs, true);
    FunctionCallee printfFunc = m.getOrInsertFunction("printf", printfType);

    Value *formatPrefix = builder.CreateGlobalStringPtr("Adiak: Name: %s, Type: %d, Value: ");
    builder.CreateCall(printfFunc, {formatPrefix, nameParam, dtypeVal});

    // switch to handle different adiak_type_t
    SwitchInst *type = builder.CreateSwitch(dtypeVal, defaultBlock, 1);
    type->addCase(builder.getInt32(ADIAK_UINT), uintCase);
    type->addCase(builder.getInt32(ADIAK_VERSION), stringCase);
    type->addCase(builder.getInt32(ADIAK_PATH), stringCase);
    type->addCase(builder.getInt32(ADIAK_STRING), stringCase);
    type->addCase(builder.getInt32(ADIAK_DATE), dateCase);


    {
        // CASE FOR ADIAK_UINT
        builder.SetInsertPoint(uintCase);

        Value *uintPtr = builder.CreateBitCast(valueParam, llvm::PointerType::getUnqual(int32Ty));
        Value *uintVal = builder.CreateLoad(int32Ty, uintPtr);

        Value *uintFmt = builder.CreateGlobalStringPtr("%u\n");
        builder.CreateCall(printfFunc, {uintFmt, uintVal});
        builder.CreateRetVoid();
    }

    {
        // CASE FOR ADIAK_STRING, ADIAK_PATH, ADIAK_VERSION
        builder.SetInsertPoint(stringCase);

        PointerType *strPtrPtrTy = llvm::PointerType::getUnqual(voidPtrTy);
        Value *strPtrPtr = builder.CreateBitCast(valueParam, strPtrPtrTy);
        Value *stringVal = builder.CreateLoad(voidPtrTy, strPtrPtr);

        Value *stringFmt = builder.CreateGlobalStringPtr("%s\n");
        builder.CreateCall(printfFunc, {stringFmt, stringVal});
        builder.CreateRetVoid();
    }

    {
        // CASE FOR ADIAK_DATE
        builder.SetInsertPoint(dateCase);

        Value *datePtr = builder.CreateBitCast(valueParam, llvm::PointerType::getUnqual(int64Ty));
        Value *dateVal = builder.CreateLoad(int64Ty, datePtr);

        Value *dateFmt = builder.CreateGlobalStringPtr("%ld\n");
        builder.CreateCall(printfFunc, {dateFmt, dateVal});
        builder.CreateRetVoid();    
    }

    builder.SetInsertPoint(defaultBlock);
    Value *unknownFmt = builder.CreateGlobalStringPtr("err: not implemented type\n");
    builder.CreateCall(printfFunc, {unknownFmt});
    builder.CreateRetVoid();

    return cb;
}
#endif

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
