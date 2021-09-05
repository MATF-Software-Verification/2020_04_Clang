/*
******                                         ******
************                             ************
Undefined value and Array Index Out Of Bounds checker
************                             ************
******                                         ******
*/

// includes
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

// namespaces
using namespace clang;
using namespace ento;
using namespace taint;


namespace {

class UndefinedValueChecker : public Checker<check::PostStmt<BinaryOperator>> {

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
void UndefinedValueChecker::reportBug(const Expr *Ex, const std::string &Msg, CheckerContext &C, std::unique_ptr<BugReporterVisitor> Visitor) const {

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

void UndefinedValueChecker::checkPostStmt(const BinaryOperator *B, CheckerContext &C) const {
  
  // check if binary operator operands are undefined, if not do not analyze statement
  if (C.getSVal(B).isUndef()) {

    // Do not report assignments of uninitialized values inside swap functions.
    // This should allow to swap partially uninitialized structs
    if (const FunctionDecl *EnclosingFunctionDecl = dyn_cast<FunctionDecl>(C.getStackFrame()->getDecl()))
      if (C.getCalleeName(EnclosingFunctionDecl) == "swap")
        return;

    // error message buffer
    std::string errMsg;

    // monitored expression
    const Expr *Ex = nullptr;

    // check is undefined left operand otherwise it's right
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

      // check if value is undefined due to array index out of bounds
      if (isArrayIndexOutOfBounds(C, Ex))
        errMsg += " due to array index out of bounds";
      
      // report found bug
      reportBug(Ex, errMsg, C);
    }


  }

}


// Register Undefined Value Checker
void ento::registerUndefinedValueChecker(CheckerManager &mgr) {
  mgr.registerChecker<UndefinedValueChecker>();
}

bool ento::shouldRegisterUndefinedValueChecker(const CheckerManager &mgr) {
  return true;
}