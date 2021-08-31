/*
******                                      ******
************                          ************
Uinitialized and Array Index Out Of Bounds checker
************                          ************
******                                      ******
*/


#include "Taint.h"
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/DynamicExtent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/ExprEngine.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace ento;
using namespace taint;



namespace {

class ExampleChecker : public Checker<check::PostStmt<BinaryOperator>> {

    private:
      mutable std::unique_ptr<BuiltinBug> BT;
      void reportBug(const Expr *Ex, const std::string &Msg, CheckerContext &C, std::unique_ptr<BugReporterVisitor> Visitor = nullptr) const;

    public:
      void checkPostStmt(const BinaryOperator *B, CheckerContext &C) const;
};

}


static const Expr *getRHSExpr(const ExplodedNode *N) {
  const Stmt *S = N->getLocationAs<PreStmt>()->getStmt();
  if (const auto *BE = dyn_cast<BinaryOperator>(S))
    return BE->getRHS();
  return nullptr;
}

// simple report bug function 
void ExampleChecker::reportBug(const Expr *Ex, const std::string &Msg, CheckerContext &C, std::unique_ptr<BugReporterVisitor> Visitor) const {

  if (ExplodedNode *N = C.generateNonFatalErrorNode()) {
    if (!BT)
      BT.reset(new BuiltinBug(this, "***BUGG***"));

    auto R = std::make_unique<PathSensitiveBugReport>(*BT, Msg.c_str(), N);

    R->addRange(Ex->getSourceRange());
    bugreporter::trackExpressionValue(N, Ex, *R);

    // R->addVisitor(std::move(Visitor));
    // bugreporter::trackExpressionValue(N, getRHSExpr(N), *R);
    C.emitReport(std::move(R));
  }

}

static bool isArrayIndexOutOfBounds(CheckerContext &C, const Expr *Ex) {
  ProgramStateRef state = C.getState();

  if (!isa<ArraySubscriptExpr>(Ex))
    return false;

  SVal Loc = C.getSVal(Ex);
  if (!Loc.isValid())
    return false;

  const MemRegion *MR = Loc.castAs<loc::MemRegionVal>().getRegion();
  const ElementRegion *ER = dyn_cast<ElementRegion>(MR);
  if (!ER)
    return false;

  DefinedOrUnknownSVal Idx = ER->getIndex().castAs<DefinedOrUnknownSVal>();
  DefinedOrUnknownSVal ElementCount = getDynamicElementCount(
      state, ER->getSuperRegion(), C.getSValBuilder(), ER->getValueType());
  ProgramStateRef StInBound = state->assumeInBound(Idx, ElementCount, true);
  ProgramStateRef StOutBound = state->assumeInBound(Idx, ElementCount, false);
  return StOutBound && !StInBound;
}

void ExampleChecker::checkPostStmt(const BinaryOperator *B, CheckerContext &C) const {
  
  // BinaryOperator::Opcode Op = B->getOpcode();

  // // Check if it is < or > op
  // if (Op == BO_GT || Op == BO_LT){
  //   reportBug("***> < ops..***", C);
  //   return;
  // }


  if (C.getSVal(B).isUndef()) {

    // Do not report insde of swap ...
    // This should allow to swap uninitialized structs
    if (const FunctionDecl *EnclosingFunctionDecl = dyn_cast<FunctionDecl>(C.getStackFrame()->getDecl()))
      if (C.getCalleeName(EnclosingFunctionDecl) == "swap")
        return;

    // err msg
    std::string errMsg;

    const Expr *Ex = nullptr;
    bool isLeft = true;

    if (C.getSVal(B->getLHS()).isUndef()) {
      Ex = B->getLHS()->IgnoreParenCasts();
      isLeft = true;
    }
    else if (C.getSVal(B->getRHS()).isUndef()) {
      Ex = B->getRHS()->IgnoreParenCasts();
      isLeft = false;
    }


     if (Ex) {
      errMsg += "The ";
      errMsg += (isLeft ? "left" : "right");
      errMsg += " operand of '";
      errMsg += BinaryOperator::getOpcodeStr(B->getOpcode());
      errMsg += "' is a garbage value";
      if (isArrayIndexOutOfBounds(C, Ex))
        errMsg += " due to array index out of bounds";
      
      reportBug(Ex, errMsg, C);
    }


  }

}


void ento::registerExampleChecker(CheckerManager &mgr) {
  mgr.registerChecker<ExampleChecker>();
}

bool ento::shouldRegisterExampleChecker(const CheckerManager &mgr) {
  return true;
}