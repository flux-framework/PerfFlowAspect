/************************************************************\
 * Copyright 2021 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Passes/OptimizationLevel.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

// TODO: Unsure about the version numbers here and below.
#if LLVM_VERSION_MAJOR > 12
#include "llvm/IR/PassManager.h"
#else
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#endif

using namespace llvm;

#if LLVM_VERSION_MAJOR > 14
bool stringRefStartsWith(StringRef S, StringRef P) { return S.starts_with(P); }
bool stringRefEndsWith(StringRef S, StringRef P) { return S.ends_with(P); }
#else
bool stringRefStartsWith(StringRef S, StringRef P) { return S.startswith(P); }
bool stringRefEndsWith(StringRef S, StringRef P) { return S.endswith(P); }
#endif

static constexpr auto WEAVER_ANNOTATION_PREFIX = "@critical_path(";
static constexpr auto WEAVER_ANNOTATION_SUFFIX = ")";

struct WeavingPassImpl {
  WeavingPassImpl(Module &M) : M(M) {}

  Module &M;
  LLVMContext &Ctx = M.getContext();
  Type *VoidTy = Type::getVoidTy(Ctx);
  Type *Int32Ty = Type::getInt32Ty(Ctx);
  Type *VoidPtrTy = PointerType::get(Type::getInt8Ty(Ctx), 0);

  FunctionType *WeaveFuncTy = FunctionType::get(
      VoidTy, {Int32Ty, VoidPtrTy, VoidPtrTy, VoidPtrTy, VoidPtrTy, VoidPtrTy},
      false);

  void createCall(StringRef RuntimeFunctionName, Instruction &I, int Async,
                  StringRef Scope, StringRef Flow, StringRef Pcut) {
    auto FC = M.getOrInsertFunction(RuntimeFunctionName, WeaveFuncTy);

    IRBuilder<> IRB(&I);
    auto *ModuleName = IRB.CreateGlobalString(M.getName());
    auto *FunctionName = IRB.CreateGlobalString(I.getFunction()->getName());
    auto *ScopeName = IRB.CreateGlobalString(Scope);
    auto *FlowName = IRB.CreateGlobalString(Flow);
    auto *PcutName = IRB.CreateGlobalString(Pcut);
    IRB.CreateCall(FC, {ConstantInt::get(Int32Ty, Async), ModuleName,
                        FunctionName, ScopeName, FlowName, PcutName});
  }

  bool insertAfter(Function &F, int Async, StringRef Scope, StringRef Flow,
                   StringRef Pcut) {
    for (auto &BB : F) {
      auto *TI = BB.getTerminator();
      if (isa<ReturnInst>(TI) || isa<ResumeInst>(TI))
        createCall("perfflow_weave_after", *TI, Async, Scope, Flow, Pcut);
    }
    return true;
  }

  bool insertBefore(Function &F, int Async, StringRef Scope, StringRef Flow,
                    StringRef Pcut) {
    createCall("perfflow_weave_before", F.getEntryBlock().front(), Async, Scope,
               Flow, Pcut);
    return true;
  }

  bool parseAnnotation(StringRef AnnotationString, StringRef &Pointcut,
                       StringRef &Scope, StringRef &Flow, bool &Async,
                       bool &Before, bool &After) {
    auto Consume = [&](StringRef Tokens, StringRef Warning) {
      AnnotationString = AnnotationString.trim();
      if (AnnotationString.consume_front(Tokens))
        return true;
      if (!Warning.empty())
        outs() << "WeavePass[WARN]: " << Warning << "\n";
      return false;
    };

    std::array<StringRef *, 3> Values = {&Pointcut, &Scope, &Flow};
    std::array<StringRef, 3> Options = {"pointcut", "scope", "flow"};
    bool Progress = true;
    while (Progress) {
      Progress = false;
      for (int Idx = 0; Idx < Options.size(); ++Idx) {
        auto Option = Options[Idx];
        AnnotationString = AnnotationString.trim();
        if (!AnnotationString.consume_front(Option))
          continue;
        Progress = true;
        if (!Consume("=", "expected \"=\" after option"))
          return false;
        if (!Consume("'", "expected \"'\" before value"))
          return false;
        AnnotationString = AnnotationString.trim();
        *Values[Idx] =
            AnnotationString.take_until([](char C) { return C == '\''; });
        Consume(*Values[Idx], "");
        if (!Consume("'", "expected \"'\" after value"))
          return false;
        if (!Consume(",", ""))
          break;
      }
      if (!Progress && !AnnotationString.empty()) {
        outs() << "Leftover annotation '" << AnnotationString
               << "', expected (X='value', Y='value', ...) with X,Y in {"
               << join(Options, ",") << "}\n";
        return false;
      }
    }

    // TODO: verify the values
    std::array<StringRef, 6> ValidValues = {"around",       "before",
                                            "after",        "around_async",
                                            "before_async", "after_async"};
    if (!any_of(ValidValues,
                [&](auto ValidValue) { return Pointcut.equals(ValidValue); })) {
      outs() << "pointcut value was " << Pointcut << " but expected any of {"
             << join(ValidValues, ",") << "}\n";
      return false;
    }

    if (stringRefStartsWith(Pointcut, "around")) {
      Before = After = true;
    } else if (stringRefStartsWith(Pointcut, "before")) {
      Before = true;
    } else if (stringRefStartsWith(Pointcut, "after")) {
      After = true;
    }
    if (stringRefEndsWith(Pointcut, "_async"))
      Async = true;

    return true;
  }

  bool run() {
    bool Changed = false;
    outs() << "WeavePass loaded successfully. \n";

    auto *AnnotationsGV = M.getNamedGlobal("llvm.global.annotations");
    if (!AnnotationsGV)
      return false;

    // Get the array of annotations.
    auto AnnotationsArray = AnnotationsGV->getInitializer();

    for (auto &OpU : AnnotationsArray->operands()) {
      // Check each element for function annotations.
      auto AnnotationStruct = cast<ConstantStruct>(OpU);
      auto *Fn = dyn_cast<Function>(
          AnnotationStruct->getOperand(0)->stripPointerCasts());
      if (!Fn)
        continue;

      // We expect global string annotations
      auto *AnnotationStringGV = dyn_cast<GlobalVariable>(
          AnnotationStruct->getOperand(1)->stripPointerCasts());
      if (!AnnotationStringGV || !AnnotationStringGV->getInitializer())
        continue;
      StringRef AnnotationString =
          cast<ConstantDataArray>(AnnotationStringGV->getInitializer())
              ->getAsCString();
      AnnotationString = AnnotationString.trim();

      // Verify the annotation is meant for us.
      if (!stringRefStartsWith(AnnotationString, WEAVER_ANNOTATION_PREFIX) ||
          !stringRefEndsWith(AnnotationString, WEAVER_ANNOTATION_SUFFIX))
        continue;
      AnnotationString.consume_front(WEAVER_ANNOTATION_PREFIX);
      AnnotationString.consume_back(WEAVER_ANNOTATION_SUFFIX);

      // Parse the values.
      StringRef Pointcut = "around", Scope = "NA", Flow = "NA";
      bool Before = false, After = false, Async = false;
      if (!parseAnnotation(AnnotationString, Pointcut, Scope, Flow, Async,
                           Before, After)) {
        errs() << "WeavePass[WARN]: Ignoring " << AnnotationString << "\n";
        continue;
      }
      outs() << "Pointcut " << Pointcut << " Scope " << Scope
             << " Flow: " << Flow << " Before " << Before << " After: " << After
             << " Async " << Async << "\n";

      // Insert runtime calls.
      if (Before)
        Changed |= insertBefore(*Fn, Async, Scope, Flow, Pointcut);
      if (After)
        Changed |= insertAfter(
            *Fn, Async, Scope,
            Before && (Flow.equals("in") || Flow.equals("out")) ? "NA" : Flow,
            Pointcut);
    }
    return Changed;
  }
};

namespace {

#if LLVM_VERSION_MAJOR > 12

struct WeavingPass : public PassInfoMixin<WeavingPass> {
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &) {
    return WeavingPassImpl(M).run() ? PreservedAnalyses::none()
                                    : PreservedAnalyses::all();
  }
  static bool isRequired() { return true; }
  static void registerPass(PassBuilder &PB) {
    PB.registerPipelineStartEPCallback(
        [](ModulePassManager &MPM, auto) { MPM.addPass(WeavingPass()); });
  }
};

extern "C" PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "weaver-pass", LLVM_VERSION_STRING,
          WeavingPass::registerPass};
}

#else

struct WeavingLegacyPass : public ModulePass {
  static char ID;
  WeavingLegacyPass() : ModulePass(ID) {}
  virtual bool runOnModule(Module &M) override {
    outs() << __PRETTY_FUNCTION__ << "\n";
    return WeavingPassImpl(M).run();
  }
};

char WeavingLegacyPass::ID = 0;

static void registerWeavingLegacyPass(const PassManagerBuilder &,
                                      legacy::PassManagerBase &PM) {
  outs() << __PRETTY_FUNCTION__ << "\n";
  PM.add(new WeavingLegacyPass());
}

static RegisterStandardPasses
    RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                   registerWeavingLegacyPass);

#endif

} // namespace
