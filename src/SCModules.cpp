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
    map<string, int> initDecl = fetchStmtInitDecl(forStmt->getInit());
    string initVal;
    if (initDecl.size() == 1) {
        for (map<string, int>::iterator i = initDecl.begin();
                i != initDecl.end(); i++) {
            _os << "init decl: \n" << i->first << " = " << i->second << "\n";
            initVal = i->first;
        }
    }

    int increment = fetchExprInc(forStmt->getInc());
    _os << "increment is: " << increment << "\n";

    Expr* cond = forStmt->getCond();
    string varName;
    int condBound;
    int opCode;
    if (BinaryOperator* bo = dyn_cast<BinaryOperator>(cond)) {
        if (Expr* lhs = bo->getLHS()) {
            if (ImplicitCastExpr* impCast = dyn_cast<ImplicitCastExpr>(lhs)) {
                for (Stmt::child_iterator it = impCast->child_begin(); 
                        it != impCast->child_end(); it++) {
                    if (DeclRefExpr* decl = dyn_cast<DeclRefExpr>(*it)) {
                        ValueDecl* vd = decl->getDecl();
                        if (vd) {
                            varName = vd->getNameAsString();
                        }
                    }
                }
            }
        }

        if (Expr* rhs = bo->getRHS()) {
            if (IntegerLiteral* literalVal = dyn_cast<IntegerLiteral>(rhs)) {
                condBound = literalVal->getValue().getSExtValue();
                _os << "condition bound: " << condBound << "\n";
            }
        }

        opCode = bo->getOpcode();
    }

    if (varName == initVal) {
        _os << "===\nForLoop Analysis:\n===\n";
        _os << "init var: " << initVal << "\n";
        _os << "init val: " << initDecl[initVal] << "\n";
        _os << "value bound: " << condBound << "\n";
        _os << "increment: " << increment << "\n";
        _os <<  "\n";
    }

}

void SCModules::printBodyDataStructs(ForStmt* forStmt)
{
    Stmt* loopBody = forStmt->getBody();
}

map<string, int> SCModules::fetchStmtInitDecl(Stmt* stmt)
{
    map<string, int> returnMap;
    if (DeclStmt* declStmt = dyn_cast<DeclStmt>(stmt)) {
        if (declStmt->isSingleDecl()) {
            Decl* singleDecl = declStmt->getSingleDecl();

            if (VarDecl* vDecl = dyn_cast<VarDecl>(singleDecl)){
                Expr* val = vDecl->getInit();

                if (IntegerLiteral* literal = dyn_cast<IntegerLiteral>(val)) {
                    returnMap.insert(make_pair(
                                vDecl->getNameAsString(), 
                                literal->getValue().getSExtValue()));
                }
            }
        }
    }

    return returnMap;
}

int SCModules::processCondExpr(Expr* expr)
{

    int opCodeNOTEQ  = 31;
    int opCodeLEQ = 32;

    _os << "ConditionExpr class:" << expr->getStmtClassName() << "\n";

    if (CXXOperatorCallExpr *ce = dyn_cast<CXXOperatorCallExpr>(expr)) {   
        _os << "CXXOperatorCallExpr opcode: " << ce->getOperator() << "\n";

        for (int i = 0; i < ce->getNumArgs(); i++) {
            _os << "CXXOPeratorCallExpr arg " << i << ":" << ce->getArg(i)->getStmtClassName() << "\n";
        }

        if (ce->getOperator() == OO_LessEqual) {
            _os << "CXXOperatorCallExpr operator is LEQ\n";
        } 
        if (ce->getOperator() == OO_ExclaimEqual) {
            _os << "CXXOperatorCallExpr operator is NOTEQ\n";
        } 

        IntegerLiteral *y = dyn_cast<IntegerLiteral>(ce->getArg(1));
        if (y) {
            _os << "CXXOperatorCallExpr y: " << y->getValue() << "\n";
        }

    } else if (BinaryOperator *bo = dyn_cast<BinaryOperator>(expr)) {   
        _os << "BinaryOperator opcode:" << bo->getOpcode() << "\n";
        if (bo->getOpcode() == BO_LT) {
            _os << "less than?\n";
        }
        Expr* lhs = bo->getLHS();
        Expr* rhs = bo->getRHS();

        _os << "BinaryOperator condition LHS: " << lhs->getStmtClassName() << "\n";
        _os << "BinaryOperator condition RHS: " << rhs->getStmtClassName() << "\n";

        if (IntegerLiteral* i = dyn_cast<IntegerLiteral>(rhs)) {
            _os << "BinaryOperator RHS of condition: " << i->getValue() << "\n";
        }
    }

}

int SCModules::fetchExprInc(Expr* expr)
{
    int retVal = 0;
    _os << "Expr class name: " << expr->getStmtClassName() << "\n";
    if (dyn_cast<BinaryOperator>(expr)) {
       BinaryOperator* binaryOperator = dyn_cast<BinaryOperator>(expr);
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

    } else if (dyn_cast<UnaryOperator>(expr)) {
        UnaryOperator* unaryOperator = dyn_cast<UnaryOperator>(expr);
       if (unaryOperator->getOpcode() == UO_PostInc || 
               unaryOperator->getOpcode() == UO_PreInc) {
           retVal = 1;
       } else if (unaryOperator->getOpcode() == UO_PostDec || 
               unaryOperator->getOpcode() == UO_PreDec) {
           retVal = -1;
       }
    }

    return retVal;
}

bool SCModules::VisitStmt(Stmt * stmt)
{
    ForStmt *forStmt = dyn_cast<ForStmt>(stmt);
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
