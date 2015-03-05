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
    // currently supports init being DeclStmt
    fetchStmtRHS(init);
    Expr* cond = forStmt->getCond();
    Expr* inc = forStmt->getInc();
    int increment = fetchExprInc(inc);
    _os << "increment is: " << increment << "\n";
}

void SCModules::printBodyDataStructs(ForStmt* forStmt)
{
    Stmt* loopBody = forStmt->getBody();
}

int SCModules::fetchStmtRHS(Stmt* stmt)
{
    if (dyn_cast_or_null<DeclStmt>(stmt)) {
        DeclStmt* declStmt = dyn_cast_or_null<DeclStmt>(stmt);
        _os << "InitStmt class name: " << declStmt->getStmtClassName() << "\n";
        if (declStmt->isSingleDecl()) {
            _os << "!! is single decl !!\n";
            Decl* singleDecl = declStmt->getSingleDecl();
            _os << "decl kind: " << singleDecl->getDeclKindName() << "\n";
        }
    }
}

int SCModules::fetchExprInc(Expr* expr)
{
    int retVal = 0;
    _os << "Expr class name: " << expr->getStmtClassName() << "\n";
    if (dyn_cast_or_null<BinaryOperator>(expr)) {
       BinaryOperator* binaryOperator = dyn_cast_or_null<BinaryOperator>(expr);
       _os << "binary class name: " << binaryOperator->getStmtClassName() << "\n";
       _os << "binary opcode: " << binaryOperator->getOpcode() << "\n";
       if (binaryOperator->getOpcode() == BO_AddAssign) {
           _os << "It's binary add operation!!\n"; 

       } else if (binaryOperator->getOpcode() == BO_SubAssign) {
           _os << "It's binary sub operation!!\n"; 
       } else if (binaryOperator->getOpcode() == BO_MulAssign) {
           _os << "It's binary mul operation!!\n"; 
       } else if (binaryOperator->getOpcode() == BO_DivAssign) {
           _os << "It's binary div operation!!\n"; 
       } else if (binaryOperator->getOpcode() == BO_XorAssign) {
           _os << "It's binary xor operation!!\n"; 
       } else if (binaryOperator->getOpcode() == BO_OrAssign) {
           _os << "It's binary or operation!!\n"; 
       } else if (binaryOperator->getOpcode() == BO_Comma) {
           _os << "It's binary comma operation!!\n"; 
           Expr* rhs = binaryOperator->getRHS();
       }

    } else if (dyn_cast_or_null<UnaryOperator>(expr)) {
        UnaryOperator* unaryOperator = dyn_cast_or_null<UnaryOperator>(expr);
       if (unaryOperator->getOpcode() == UO_PostInc || 
               unaryOperator->getOpcode() == UO_PreInc) {
           _os << "post/pre inc!!!\n";
           retVal = 1;
       } else if (unaryOperator->getOpcode() == UO_PostDec || 
               unaryOperator->getOpcode() == UO_PreDec) {
           _os << "post/pre dec!!!\n";
           retVal = -1;
       }
    }

    return retVal;
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
