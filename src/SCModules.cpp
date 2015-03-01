#include "SCModules.h"
#include "FindModule.h"
using namespace scpar;

SCModules::SCModules(TranslationUnitDecl * tuDecl, llvm::raw_ostream & os):
_os(os)
{
  assert(!(tuDecl == NULL));
  TraverseDecl(tuDecl);
}

void SCModules::printLoopBounds(ForStmt* forStmt)
{
    Stmt* init = forStmt->getInit();
    Expr* cond = forStmt->getCond();
    Expr* inc = forStmt->getInc();
}

void SCModules::printBodyDataStructs(ForStmt* forStmt)
{
    Stmt* loopBody = forStmt->getBody();
}

bool SCModules::VisitStmt(Stmt * stmt)
{
    ForStmt *forStmt = dyn_cast_or_null<ForStmt>(stmt);
    if (forStmt) {
        _os << "VisitStmt class name: " << stmt->getStmtClassName() << "\n";

        printLoopBounds(forStmt);

        printBodyDataStructs(forStmt);
    }

    return true;
}

bool SCModules::VisitCXXRecordDecl(CXXRecordDecl * cxxDecl)
{
  FindModule mod(cxxDecl, _os);

  if (!mod.isSystemCModule()) {
    return true;
  }
  string modName = mod.getModuleName();
  _moduleMap.insert(modulePairType(modName, cxxDecl));

  TraverseStmt(cxxDecl->getBody());

  return true;
}

SCModules::moduleMapType SCModules::getSystemCModulesMap()
{
  return _moduleMap;
}

void SCModules::printSystemCModulesMap()
{
  _os << "\n================= SCModules ================";
  _os << "\n Print SystemC Module Map";
  for (moduleMapType::iterator mit = _moduleMap.begin();
       mit != _moduleMap.end(); mit++) {
    _os << "\n:> name: " << mit->first << ", CXXRecordDecl*: " << mit->second;
  }
  _os << "\n================= END SCModules ================";
  _os << "\n\n";
}
