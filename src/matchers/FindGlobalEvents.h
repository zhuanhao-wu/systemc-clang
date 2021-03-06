#ifndef _FIND_GLOBAL_EVENTS_H_
#define _FIND_GLOBAL_EVENTS_H_
#include "systemc-clang.h"
#include "json.hpp"

#include "clang/AST/Decl.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "llvm/Support/raw_ostream.h"
#include <map>
#include <string>
#include <vector>

namespace systemc_clang {
using namespace clang;
using namespace std;
 using json = nlohmann::json;
 
class FindGlobalEvents : public RecursiveASTVisitor<FindGlobalEvents> {
public:
  typedef map<string, VarDecl *> globalEventMapType;
  typedef pair<string, VarDecl *> kvType;

  FindGlobalEvents(TranslationUnitDecl *, llvm::raw_ostream &);
  virtual ~FindGlobalEvents();

  virtual bool VisitVarDecl(VarDecl *);

  globalEventMapType getEventMap();
  vector<string> getEventNames();

  void dump();
  json dump_json();


private:
  llvm::raw_ostream &_os;
  globalEventMapType _globalEvents;
};
} // namespace systemc_clang
#endif
