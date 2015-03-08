#include "FindGPUMacro.h"
using namespace scpar;

GPUMacro::GPUMacro(int blockIdx, int blockIdy, int blockIdz, int threadIdx, int threadIdy, int threadIdz, int gpuTime, int cpuTime) :
  _blockIdx(blockIdx),
  _blockIdy(blockIdy),
  _blockIdz(blockIdz),
  _threadIdx(threadIdx),
  _threadIdy(threadIdy),
  _threadIdz(threadIdz),
  _gpuTime(gpuTime),
  _cpuTime(cpuTime),
	_gpuFit(false){
  }

GPUMacro::GPUMacro():
  _blockIdx(1),
  _blockIdy(1),
  _blockIdz(1),
  _threadIdx(1),
  _threadIdy(1),
  _threadIdz(1),
  _gpuTime(0),
  _cpuTime(0),
	_gpuFit(false){
					
  
  }

GPUMacro::~GPUMacro() {
}

void GPUMacro::addGPUFit() {
	_gpuFit = true;
}

void GPUMacro::denyGPUFit() {
	_gpuFit = false;
}

bool GPUMacro::isGPUFit() {
	return _gpuFit;
}

int GPUMacro::getBlockIdx(){
 return _blockIdx;
}

int GPUMacro::getBlockIdy(){
 return _blockIdy;
}


int GPUMacro::getBlockIdz(){
 return _blockIdz;
}


int GPUMacro::getThreadIdx(){
 return _threadIdx;
}


int GPUMacro::getThreadIdy(){
 return _threadIdx;
}


int GPUMacro::getThreadIdz(){
 return _threadIdz;
}

int GPUMacro::getCPUTime() {
 return _cpuTime;
}

int GPUMacro::getGPUTime() {
 return _gpuTime;
}

void GPUMacro::dump(raw_ostream& os) {
 os <<"\n Block Ids : " <<_blockIdx<<" " <<_blockIdy<<" " <<_blockIdz;

 os <<"\n Thread Ids : " <<_threadIdx<<" " <<_threadIdy<<" " <<_threadIdz;
 
 os <<"\n GPU time : " <<_gpuTime<<" CPU time : " <<_cpuTime;
}

///////////////////////////////////////////////////////
FindGPUMacro::FindGPUMacro(CXXMethodDecl* entryFunction, int instanceNum, llvm::raw_ostream& os):
  _entryFunction(entryFunction),
  _instanceNum(instanceNum),
	_os(os)
{
 TraverseDecl(_entryFunction);
}

FindGPUMacro::~FindGPUMacro()
{
}

bool FindGPUMacro::VisitForStmt(ForStmt *fstmt) {
 
  Stmt *body = fstmt->getBody();
  int tx = 1, ty = 1, tz = 1 , bx = 1, by = 1, bz = 1, gpu_time = 0, cpu_time = 0, instanceNum = 0;
  for (Stmt::child_iterator it = body->child_begin(), eit = body->child_end();
      it != eit;
      it++) {

    Stmt *s = *it; 

    if (DeclStmt *ds = dyn_cast<DeclStmt>(s)){
      if (VarDecl *vd = dyn_cast<VarDecl>(ds->getSingleDecl())){
        string className = vd->getTypeSourceInfo()->getType().getBaseTypeIdentifier()->getName();
				if (className == "profile_time") {
	 				if (CXXConstructExpr *ce = dyn_cast<CXXConstructExpr>(vd->getInit()->IgnoreImpCasts())) {					 
	 	 				if (MaterializeTemporaryExpr *me = dyn_cast<MaterializeTemporaryExpr>(ce->getArg(0)->IgnoreImpCasts())) {
							if (CXXTemporaryObjectExpr *co = dyn_cast<CXXTemporaryObjectExpr>(me->GetTemporaryExpr()->IgnoreImpCasts())) {

								IntegerLiteral *x = dyn_cast<IntegerLiteral>(co->getArg(0));
								IntegerLiteral *y = dyn_cast<IntegerLiteral>(co->getArg(1));
	  						IntegerLiteral *z = dyn_cast<IntegerLiteral>(co->getArg(2));

								instanceNum = x->getValue().getSExtValue();
	  						gpu_time = y->getValue().getSExtValue();
	  						cpu_time = z->getValue().getSExtValue();
	 						}	 
						}
				}
		}	
		
    if (className == "sc_gpu_thread_hierarchy") {   
    	if (CXXConstructExpr *ce = dyn_cast<CXXConstructExpr>(vd->getInit()->IgnoreImpCasts())) {
				if (MaterializeTemporaryExpr *me = dyn_cast<MaterializeTemporaryExpr>(ce->getArg(0)->IgnoreImpCasts())) {
					if (CXXTemporaryObjectExpr *co = dyn_cast<CXXTemporaryObjectExpr>(me->GetTemporaryExpr()->IgnoreImpCasts())) {
          	IntegerLiteral *x = dyn_cast<IntegerLiteral>(co->getArg(1)); 
          	IntegerLiteral *y = dyn_cast<IntegerLiteral>(co->getArg(2));
          	IntegerLiteral *z = dyn_cast<IntegerLiteral>(co->getArg(3)); 
						IntegerLiteral *w = dyn_cast<IntegerLiteral>(co->getArg(4));
          	instanceNum = x->getValue().getSExtValue();
						tx = x->getValue().getSExtValue();
          	ty = y->getValue().getSExtValue();
          	tz = z->getValue().getSExtValue();          
         	}        
        }
			}
		}
    if (className == "sc_gpu_block_hierarchy") {   
    	if (CXXConstructExpr *ce = dyn_cast<CXXConstructExpr>(vd->getInit()->IgnoreImpCasts())) {
				if (MaterializeTemporaryExpr *me = dyn_cast<MaterializeTemporaryExpr>(ce->getArg(0)->IgnoreImpCasts())) {
					if (CXXTemporaryObjectExpr *co = dyn_cast<CXXTemporaryObjectExpr>(me->GetTemporaryExpr()->IgnoreImpCasts())) {
          	IntegerLiteral *x = dyn_cast<IntegerLiteral>(co->getArg(1)); 
          	IntegerLiteral *y = dyn_cast<IntegerLiteral>(co->getArg(2));
          	IntegerLiteral *z = dyn_cast<IntegerLiteral>(co->getArg(3)); 
          	IntegerLiteral *w = dyn_cast<IntegerLiteral>(co->getArg(4));
				  
						instanceNum = x->getValue().getSExtValue();	
						bx = y->getValue().getSExtValue();
          	by = z->getValue().getSExtValue();
          	bz = w->getValue().getSExtValue();           
         			}        
        		}
					}
				}
  		}
		}
	
		//_os <<"\n gpu_time : " <<gpu_time<<" cpu_time : " <<cpu_time<<" instanceNum : " <<_instanceNum<<" " <<instanceNum;
  	if (tx && ty && tz && bx && by && bz && gpu_time && cpu_time && (_instanceNum == instanceNum)) {
			//_os <<"\n instance num : " <<_instanceNum<<" " <<instanceNum;
  	  GPUMacro *gm = new GPUMacro(bx, by, bz, tx, ty, tz, gpu_time, cpu_time);
			//_os <<"\n for stmt : " <<fstmt;
			forStmtInstanceIdPairType forStmtInstanceId = make_pair(_instanceNum, fstmt);
			_forStmtGPUMacroMap.insert(forStmtGPUMacroPairType(forStmtInstanceId, gm)); 			
			break;
		}
	}
  return true;
}

int FindGPUMacro::fetchExprInc(Expr* expr) {
    int retVal = 0;
    bool negative = false;
    if (dyn_cast<BinaryOperator>(expr)) {
        BinaryOperator* binaryOperator = dyn_cast<BinaryOperator>(expr);
        bool isSubOperation = false;

        switch (binaryOperator->getOpcode()) {
            case BO_SubAssign:
                isSubOperation = true;
                break;
            case BO_AddAssign:
                _os << "It's binary AddAssign operation!!\n"; 
                break; 

            default:
                break;
        }
        int incrValue = 0;
        if (Expr* rhs = binaryOperator->getRHS()) {
            incrValue = extractValueFromIntegerLiteral(rhs);
        }
        if (isSubOperation) incrValue *= -1;

        return incrValue;

    } else if (dyn_cast<UnaryOperator>(expr)) {
        UnaryOperator* unaryOperator = dyn_cast<UnaryOperator>(expr);
        if (unaryOperator->getOpcode() == UO_PostInc || 
                unaryOperator->getOpcode() == UO_PreInc) {
            return 1;
        } else if (unaryOperator->getOpcode() == UO_PostDec || 
                unaryOperator->getOpcode() == UO_PreDec) {
            return -1;
        }
    } 

    return retVal;

}

int FindGPUMacro::processCondExpr(Expr* expr) {
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

void FindGPUMacro::printLoopBounds(ForStmt* forStmt) {
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
            condBound = extractValueFromIntegerLiteral(rhs);
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

int FindGPUMacro::extractValueFromIntegerLiteral(Expr* expr) {
    if (IntegerLiteral* literalVal = dyn_cast<IntegerLiteral>(expr)) {
        return literalVal->getValue().getSExtValue();

    } else if (UnaryOperator* uo = dyn_cast<UnaryOperator>(expr)) {
        if (IntegerLiteral* literalVal = dyn_cast<IntegerLiteral>(uo->getSubExpr())) {
            return -1 * literalVal->getValue().getSExtValue();
        }
    }
}

FindGPUMacro::forStmtGPUMacroMapType FindGPUMacro::getForStmtGPUMacroMap() {
 
  return _forStmtGPUMacroMap;
}

void FindGPUMacro::dump(){
 _os <<"\n For Stmt GPU Macro";
 for (forStmtGPUMacroMapType::iterator it = _forStmtGPUMacroMap.begin(), eit = _forStmtGPUMacroMap.end();
     it != eit;
     it++) {
  
   _os <<"\n For Stmt : " <<it->first.second;
   _os <<"\n GPU kernel parameters \n";
   it->second->dump(_os);
 }
}
