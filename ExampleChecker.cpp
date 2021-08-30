/*

CHECKER...


*/



#include "Taint.h"
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace clang;
using namespace ento;
using namespace taint;



namespace {

class ExampleChecker : public Checker<check::PreStmt<BinaryOperator>> {

    private:
      mutable std::unique_ptr<BuiltinBug> BT;
      void reportBug(const std::string &Msg, CheckerContext &C, std::unique_ptr<BugReporterVisitor> Visitor = nullptr) const;

    public:
      void checkPreStmt(const BinaryOperator *B, CheckerContext &C) const;
};

}


static const Expr *getRHSExpr(const ExplodedNode *N) {
  const Stmt *S = N->getLocationAs<PreStmt>()->getStmt();
  if (const auto *BE = dyn_cast<BinaryOperator>(S))
    return BE->getRHS();
  return nullptr;
}

// simple report bug funstion
void ExampleChecker::reportBug(const std::string &Msg, CheckerContext &C, std::unique_ptr<BugReporterVisitor> Visitor) const {

  if (ExplodedNode *N = C.generateNonFatalErrorNode()) {
    if (!BT)
      BT.reset(new BuiltinBug(this, "***BUGG***"));

    auto R = std::make_unique<PathSensitiveBugReport>(*BT, Msg.c_str(), N);
    R->addVisitor(std::move(Visitor));
    bugreporter::trackExpressionValue(N, getRHSExpr(N), *R);
    C.emitReport(std::move(R));
  }

}

void ExampleChecker::checkPreStmt(const BinaryOperator *B, CheckerContext &C) const {
  
  BinaryOperator::Opcode Op = B->getOpcode();

  // Check if it is < or > op
  if (Op == BO_GT || Op == BO_LT){
    reportBug("***> < ops..***", C);
    return;
  }

}


void ento::registerExampleChecker(CheckerManager &mgr) {
  mgr.registerChecker<ExampleChecker>();
}

bool ento::shouldRegisterExampleChecker(const CheckerManager &mgr) {
  return true;
}