//===-- src/EntryFunctionContainer.h - systec-clang class definition -------*-
// C++ -*-===//
//
//                     systemc-clang: SystemC Parser
//
// This file is distributed under the University of Illinois License.
// See LICENSE.mkd for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief A container class to hold entry function information.
///
//===----------------------------------------------------------------------===//
#ifndef _ENTRY_FUNCTION_CONTAINER_H_
#define _ENTRY_FUNCTION_CONTAINER_H_

#include <map>
#include <string>
#include "FindNotify.h"
//#include "FindSensitivity.h"
#include "SensitivityMatcher.h"
#include "FindWait.h"
#include "NotifyContainer.h"
#include "SuspensionAutomata.h"
#include "Utility.h"
#include "WaitContainer.h"
#include "clang/AST/DeclCXX.h"
#include "enums.h"
namespace scpar {

using namespace clang;
using namespace std;

class EntryFunctionContainer {
 public:
  // typedefs
  typedef vector<WaitContainer *> waitContainerListType;
  typedef vector<NotifyContainer *> notifyContainerListType;

  typedef pair<int, SuspensionAutomata::transitionVectorType>
      instanceSautoPairType;
  typedef map<int, SuspensionAutomata::transitionVectorType>
      instanceSautoMapType;

  typedef pair<int, SuspensionAutomata::susCFGVectorType>
      instanceSusCFGPairType;
  typedef map<int, SuspensionAutomata::susCFGVectorType> instanceSusCFGMapType;

  // Sensitivity information
  typedef std::tuple<std::string, ValueDecl *, MemberExpr *>
      SensitivityTupleType;
  typedef std::pair<std::string, std::vector<SensitivityTupleType>>
      SensitivityPairType;

  typedef std::map<std::string, std::vector<SensitivityTupleType>> SenseMapType;

  EntryFunctionContainer();
  EntryFunctionContainer(string, PROCESS_TYPE, CXXMethodDecl *, Stmt *);
  EntryFunctionContainer(const EntryFunctionContainer &);
  ~EntryFunctionContainer();

  // Accessors.
  string getName();
  CXXMethodDecl *getEntryMethod();
  // Stmt *getConstructorStmt ();
  PROCESS_TYPE getProcessType();

  waitContainerListType getWaitCalls();
  notifyContainerListType getNotifyCalls();
  SenseMapType getSenseMap();
  SuspensionAutomata::susCFGVectorType getSusCFG();
  SuspensionAutomata::transitionVectorType getSusAuto();
  instanceSautoMapType getInstanceSautoMap();
  instanceSusCFGMapType getInstanceSusCFGMap();

  // Add waits.
  //void addSensitivityInfo(FindSensitivity &);
  void addSensitivityInfo(SenseMapType &);
  void addWaits(FindWait &);
  void addNotifys(FindNotify &);
  void addSusCFGAuto(SuspensionAutomata &);
  // void setConstructorStmt (Stmt *);
  void setName(string);
  void setProcessType(PROCESS_TYPE);
  void setEntryMethod(CXXMethodDecl *);
  void dumpSusCFG(raw_ostream &);
  void dumpSauto(raw_ostream &);
  void dump(raw_ostream &, int);

  // private:
  string _entryName;
  PROCESS_TYPE _procType;
  CXXMethodDecl *_entryMethodDecl;

  // Hold all the waits.
  waitContainerListType _waitCalls;
  notifyContainerListType _notifyCalls;

  // Sensitivity information
  //FindSensitivity::senseMapType _senseMap;
  SenseMapType senseMap_;

  vector<Transition *> _susAuto;
  vector<SusCFG *> _susCFG;
};
}  // namespace scpar
#endif
