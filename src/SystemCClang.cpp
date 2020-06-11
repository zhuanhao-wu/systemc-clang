#include "SystemCClang.h"

#include "Matchers.h"
#include "SensitivityMatcher.h"

#include "ModuleInstanceType.h"
#include "NetlistMatcher.h"

#include "clang/AST/ASTImporter.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace scpar;
using namespace clang;
using namespace std;

using namespace sc_ast_matchers;

// Private methods
void SystemCConsumer::populateNestedModules(
    const InstanceMatcher::InstanceDeclarations &instance_map) {
  // Decl* => ModuleInstanceType
  for (auto const &inst : instance_map) {
    // get<0>(inst) is the Decl*, and get<1>(inst) is the ModuleInstanceType
    ModuleInstanceType module_inst{get<1>(inst)};
    module_inst.dump();
    // Match with the same InstanceTypeDecl
    ModuleDecl *child{
        systemcModel_->getInstance(module_inst.getInstanceDecl())};
    llvm::outs() << "# child instance decl " << module_inst.getInstanceDecl()
                 << "\n";
    module_inst.getInstanceDecl()->dump();

    ModuleDecl *parent{systemcModel_->getInstance(module_inst.getParentDecl())};

    if (child) {
      llvm::outs() << "- child " << child->getName() << " : "
                   << child->getInstanceName() << "\n";
    }
    if (parent) {
      llvm::outs() << "- parent " << parent->getName() << " : "
                   << parent->getInstanceName() << "\n";
    }

    // Insert the child into the parent.
    if (child && parent) {
      llvm::outs() << "ADD A CHILD PARENT RELATIONSHIP\n";
      parent->addNestedModule(child);
    }
  }
}

bool SystemCConsumer::preFire() { return true; }

bool SystemCConsumer::postFire() { return true; }

bool SystemCConsumer::fire() {
  os_ << "Top module: " << getTopModule() << "\n";
  TranslationUnitDecl *tu{getContext().getTranslationUnitDecl()};
  // Reflection database.
  systemcModel_ = new Model{};

  // ANI : Do we need FindGlobalEvents?
  FindGlobalEvents globals{tu, os_};
  FindGlobalEvents::globalEventMapType eventMap{globals.getEventMap()};
  // globals.dump_json();
  systemcModel_->addGlobalEvents(eventMap);

  //
  // TODO:
  // A first pass should be made to collect all sc_module declarations.
  // This is important so that the top-level module can be found.
  //

  // A first pass should be made to collect all sc_module declarations.
  // This is important so that the top-level module can be found.
  //
  os_ << "================ TESTMATCHER =============== \n";

  // InstanceMatcher match_instances{};
  ModuleDeclarationMatcher module_declaration_handler{};
  module_declaration_handler.set_top_module_decl(getTopModule());
  MatchFinder matchRegistry{};

  // Run all the matchers

  module_declaration_handler.registerMatchers(matchRegistry);
  // match_instances.registerMatchers( matchRegistry );

  matchRegistry.matchAST(getContext());
  // match_instances.dump();
  llvm::outs() << "=================== END MATCHER ====================";

  module_declaration_handler.pruneMatches();
  os_ << "== Print module declarations pruned\n";
  module_declaration_handler.dump();
  os_ << "================ END =============== \n";

  // Check if the top-level module one of the sc_module declarations?
  //
  // Map CXXRecordDecl => ModuleDecl*
  auto found_module_declarations{
      module_declaration_handler.getFoundModuleDeclarations()};
  auto found_top_module{std::find_if(
      found_module_declarations.begin(), found_module_declarations.end(),
      [this](const ModuleDeclarationMatcher::ModulePairType &element) {
        return (element.second->getName() == getTopModule());
      })};

  if (found_top_module != found_module_declarations.end()) {
    os_ << "Found the top module: " << found_top_module->second->getName()
        << ", " << found_top_module->second << "\n";
  }

  // ===========================================================
  // 1. Add every module declaration to the model.
  // ===========================================================
  // Every module declaration that is found should be added to the model.
  //

  /*
  for (auto const &element : found_module_declarations) {
    auto module_declaration{new ModuleDecl{get<0>(element), get<1>(element)}};
    systemcModel_->addModuleDecl(module_declaration);
  }
  */

  ////////////////////////////////////////////////////////////////
  // Find the sc_main
  ////////////////////////////////////////////////////////////////
  FindSCMain scmain{tu, os_};

  if (scmain.isSCMainFound()) {
    FunctionDecl *fnDecl{scmain.getSCMainFunctionDecl()};

    // TODO: find any instances in sc_main.

    FindSimTime scstart{fnDecl, os_};
    systemcModel_->addSimulationTime(scstart.returnSimTime());
  } else {
    os_ << "\n Could not find SCMain";
  }

  ////////////////////////////////////////////////////////////////
  // Find the netlist.
  ////////////////////////////////////////////////////////////////
  // This actually also finds instances, but now we have AST matchers to do it.
  //
  //  TEST NetlistMatcher
  //  map: CXXRecordDecl =>  InstanceListType
  //  -> InstanceListType: vector<InstanceMatcher::InstanceDeclType>
  //  -> InstanceMatcher::InstanceDeclType :  std::tuple<std::string, Decl *,
  //  ModuleInstanceType>
  auto found_instances_declaration_map{
      module_declaration_handler.getInstances()};
  //
  // Create a ModuleDecl for each instance with the appropriately parsed
  // ModuleDecl.
  //

  // Go through each instance and find its appropriate module declaration.
  for (const auto &inst : found_instances_declaration_map) {
    // This is the CXXRecordDecl
    auto cxx_decl{inst.first};
    // List of InstanceListType
    auto instance_list{inst.second};

    auto incomplete_module_decl{found_module_declarations[cxx_decl]};
    // TODO:
    //
    // FIXME: This has to be replaced once xlat is fixed.
    std::vector<ModuleDecl *> module_decl_instances;
    // ModuleDecl *p_dummy_module_decl{incomplete_module_decl};

    for (const auto &instance : instance_list) {
      auto add_module_decl{new ModuleDecl{*incomplete_module_decl}};
      // 0: string
      // 1: Decl* (InstaceTypeDecl)
      // 2: ModuleInstanceType

      // Insert what you know about the parsed sc_module
      // 1. Insert the instance name from Matchers
      //
      auto inst_info{get<2>(instance)};
      inst_info.dump();

      os_ << "\n";
      os_ << "1. Set instance name: " << get<0>(instance) << "\n";
      // add_module_decl->setInstanceName(get<0>(instance));
      os_ << "2. Set instance decl: " << cxx_decl->getNameAsString() << " "
          << inst_info.getInstanceDecl() << "\n";
      add_module_decl->setInstanceInfo(get<2>(instance));

      // 2. Find the template arguments for the class.
      // In clang lingo: parameters are the templated values, and the arguments
      // are the specialization values for the templates.
      //
      os_ << "3. Set template arguments\n";
      FindTemplateParameters tparms{cxx_decl, os_};
      add_module_decl->setTemplateParameters(tparms.getTemplateParameters());
      add_module_decl->setTemplateArgs(tparms.getTemplateArgs());

      // 3. Find constructor
      //
      //
      os_ << "4. Set the constructor.\n";
      vector<EntryFunctionContainer *> _entryFunctionContainerVector;
      FindConstructor constructor{add_module_decl->getModuleClassDecl(), os_};
      add_module_decl->addConstructor(&constructor);
      // add_module_decl->addConstructor(constructor.getConstructorStmt());

      // 4. Find ports
      // This is done for the declaration.
      //
      //
      // 5. Find  entry functions
      os_ << "5. Set the entry functions\n";
      FindEntryFunctions findEntries{add_module_decl->getModuleClassDecl(),
                                     os_};
      FindEntryFunctions::entryFunctionVectorType *entryFunctions{
          findEntries.getEntryFunctions()};
      os_ << "6. Set the process\n";
      add_module_decl->addProcess(entryFunctions);

      //add_module_decl->dump(llvm::outs());

      for (size_t i{0}; i < entryFunctions->size(); i++) {
        EntryFunctionContainer *ef{(*entryFunctions)[i]};
        //FindSensitivity findSensitivity{constructor.getConstructorStmt(), os_};
        llvm::outs()
            << "\n================ SENSITIVITY MATCHER =============== \n";
        SensitivityMatcher sens_matcher{};
        MatchFinder matchRegistry{};
        sens_matcher.registerMatchers(matchRegistry);
        // Run all the matchers
        matchRegistry.match(*constructor.getConstructorDecl(), getContext());
        sens_matcher.dump();
        llvm::outs() << "================ END =============== \n";

        //ef->addSensitivityInfo(findSensitivity);
        EntryFunctionContainer::SenseMapType sensitivity_info{sens_matcher.getSensitivityMap()};
        ef->addSensitivityInfo(sensitivity_info);

        if (ef->getEntryMethod() == nullptr) {
          os_ << "ERROR";
          continue;
        }

        FindWait findWaits{ef->getEntryMethod(), os_};
        ef->addWaits(findWaits);

        FindNotify findNotify{ef->_entryMethodDecl, os_};
        ef->addNotifys(findNotify);

        _entryFunctionContainerVector.push_back(ef);
      }

      // Insert the module into the model.
      // All modules are also instances.

      // Make the dummy equal to the updated add_module_decl
      // This will make module declarations be one of the module instances.
      //*p_dummy_module_decl = *add_module_decl;
      // systemcModel_->addModuleDecl(p_dummy_module_decl);
      module_decl_instances.push_back(add_module_decl);
    }
    os_ << "\n";

    // TODO:
    //
    // FIXME: Only there to make sure xlat still compiles.
    // This should be removed.
    llvm::outs() << "[HDP] Add instances to model\n";
    systemcModel_->addModuleDeclInstances(incomplete_module_decl,
                                          module_decl_instances);
  }

  // Module instance map.
  auto module_instance_map{systemcModel_->getModuleInstanceMap()};

  llvm::outs()
      << " @@@@@@@@ =============== Populate sub-modules ============= \n";
  // This must have the instance matcher already run.
  // You need systemcModel_ and instance_matcher to build the hierarchy of
  // sub-modules.
  auto instance_matcher{module_declaration_handler.getInstanceMatcher()};
  auto instance_map{instance_matcher.getInstanceMap()};
  llvm::outs() << "- Print out all the instances in the instance map\n";
  populateNestedModules(instance_map);

  llvm::outs() << "===========END  Populate sub-modules ============= \n";

  // All instances are within the SystemC model.
  //  This must come after instances of ModuleDecl have been generated.
  //  This is because the netlist matcher inserts the port bindings into the
  //  instance.

  llvm::outs() << "=============== ##### TEST NetlistMatcher ##### \n";
  NetlistMatcher netlist_matcher{};
  MatchFinder netlist_registry{};
  netlist_matcher.registerMatchers(netlist_registry, systemcModel_,
                                   &module_declaration_handler);

  // scmain.getSCMainFunctionDecl()->dump();

  netlist_registry.match(*scmain.getSCMainFunctionDecl(), getContext());
  // TODO: Fix the top-level
  if (getTopModule() == "!none") {
    llvm::outs() << " No top module\n";
  }

  llvm::outs() << "Begin netlist parsing on instances: "
               << found_instances_declaration_map.size() << "\n";
  for (const auto &inst : module_instance_map) {
    auto incomplete_mdecl{inst.first};
    auto instance_list{inst.second};

    for (auto const &instance : instance_list) {
      // ModuleDecl *mdecl{systemcModel_->getInstance(get<0>(instance))};
      ModuleDecl *mdecl{instance};
      auto ctordecl{mdecl->getConstructorDecl()};
      if (ctordecl != nullptr) {
        const FunctionDecl *fd{dyn_cast<FunctionDecl>(ctordecl)};
        ctordecl->getBody(fd);
        llvm::outs() << "==============> RUN netlist matcher\n";
        fd->dump();
        netlist_registry.match(*fd, getContext());
        llvm::outs() << "==============> DONE netlist matcher\n";
      }
    }
  }
  netlist_matcher.dump();
  llvm::outs() << "##### END TEST NetlistMatcher ##### \n";

  /*
  FindNetlist findNetlist{scmain.getSCMainFunctionDecl()};
  findNetlist.dump();
  systemcModel_->addNetlist(findNetlist);
  */

  /*
  ////////////////////////////////////////////////////////////////
  // Figure out the module map.
  ////////////////////////////////////////////////////////////////
  Model::moduleMapType moduleMap{systemcModel_->getModuleDecl()};

  // <string, ModuleDecl*>
  for (Model::moduleMapType::iterator mit = moduleMap.begin(),
                                      mitend = moduleMap.end();
       mit != mitend; mit++) {
    ModuleDecl *mainmd{mit->second};
    int numInstances{mainmd->getNumInstances()};
    vector<ModuleDecl *> moduleDeclVec;

    os_ << "\nFor module: " << mit->first << " num instance : " << numInstances
        << "\n";

    for (unsigned int num{0}; num < numInstances; ++num) {
      auto md{new ModuleDecl{*mainmd}};

      // Find the template arguments for the class.
      // FindTemplateParameters tparms{mainmd->getModuleClassDecl(), os_};

      // md->setTemplateParameters(tparms.getTemplateParameters());
      // md->dump_json();

      vector<EntryFunctionContainer *> _entryFunctionContainerVector;
      FindConstructor constructor{mainmd->getModuleClassDecl(), os_};
      md->addConstructor(constructor.returnConstructorStmt());

      FindPorts ports{mainmd->getModuleClassDecl(), os_};
      //      ports.dump();
      md->addInputPorts(ports.getInputPorts());
      md->addOutputPorts(ports.getOutputPorts());
      md->addInputOutputPorts(ports.getInputOutputPorts());
      md->addOtherVars(ports.getOtherVars());
      md->addInputStreamPorts(ports.getInStreamPorts());
      md->addOutputStreamPorts(ports.getOutStreamPorts());

      FindTLMInterfaces findTLMInterfaces{mainmd->getModuleClassDecl(), os_};
      md->addInputInterfaces(findTLMInterfaces.getInputInterfaces());
      md->addOutputInterfaces(findTLMInterfaces.getOutputInterfaces());
      md->addInputOutputInterfaces(
          findTLMInterfaces.getInputOutputInterfaces());

      FindSignals signals{mainmd->getModuleClassDecl(), os_};
      md->addSignals(signals.getSignals());

      FindEntryFunctions findEntries{mainmd->getModuleClassDecl(), os_};
      FindEntryFunctions::entryFunctionVectorType *entryFunctions{
          findEntries.getEntryFunctions()};
      md->addProcess(entryFunctions);

      for (size_t i = 0; i < entryFunctions->size(); i++) {
        EntryFunctionContainer *ef{(*entryFunctions)[i]};
        FindSensitivity findSensitivity{constructor.returnConstructorStmt(),
                                        os_};
        ef->addSensitivityInfo(findSensitivity);

        if (ef->getEntryMethod() == nullptr) {
          os_ << "ERROR";
          continue;
        }

        FindWait findWaits{ef->getEntryMethod(), os_};
        ef->addWaits(findWaits);

        FindNotify findNotify{ef->_entryMethodDecl, os_};
        ef->addNotifys(findNotify);

#ifdef USE_SAUTO
        /// Does not compile
        SuspensionAutomata suspensionAutomata(findWaits.getWaitCalls(),
                                              ef->getEntryMethod(),
                                              &getContext(), llvm::errs());
        if (suspensionAutomata.initialize()) {
          suspensionAutomata.genSusCFG();
          suspensionAutomata.dumpSusCFG();
          suspensionAutomata.genSauto();
          suspensionAutomata.dumpSauto();
          ef->addSusCFGAuto(suspensionAutomata);
        }
#endif

        _entryFunctionContainerVector.push_back(ef);
      }
      moduleDeclVec.push_back(md);
    }
    // systemcModel_->addModuleDeclInstances(mainmd, moduleDeclVec);
  }
  */

  /*
     FindSCMain scmain(tu, os_);

     if (scmain.isSCMainFound())
     {
     FunctionDecl *fnDecl = scmain.getSCMainFunctionDecl();

     FindSimTime scstart(fnDecl, os_);
     systemcModel_->addSimulationTime(scstart.returnSimTime());

     }
     else {
     os_ <<"\n Could not find SCMain";
     }

     FindNetlist findNetlist(scmain.getSCMainFunctionDecl());
     findNetlist.dump();
     systemcModel_->addNetlist(findNetlist);

  // Only do this if SAUTO flag is set.
#ifdef USE_SAUTO
  // Generate SAUTO
  // Placing it here so that unique SAUTO for each instance
  // Model::moduleMapType moduleMap = systemcModel_->getModuleDecl();
  Model::moduleInstanceMapType moduleInstanceMap =
      systemcModel_->getModuleInstanceMap();

  for (Model::moduleInstanceMapType::iterator it = moduleInstanceMap.begin(),
                                              eit = moduleInstanceMap.end();
       it != eit; it++) {
    vector<ModuleDecl *> moduleDeclVec = it->second;
    for (size_t i = 0; i < moduleDeclVec.size(); i++) {
      ModuleDecl *moduleDecl = moduleDeclVec.at(i);
      vector<EntryFunctionContainer *> entryFunctionContainer =
          moduleDecl->getEntryFunctionContainer();
      for (size_t j = 0; j < entryFunctionContainer.size(); j++) {
        SuspensionAutomata suspensionAutomata(
            entryFunctionContainer.at(j)->getWaitCalls(),
            entryFunctionContainer.at(j)->getEntryMethod(), &getContext(),
            llvm::errs());
        if (suspensionAutomata.initialize()) {
          suspensionAutomata.genSusCFG();
          // suspensionAutomata.dumpSusCFG();
          suspensionAutomata.genSauto();
          // suspensionAutomata.dumpSauto();
          entryFunctionContainer.at(j)->addSusCFGAuto(suspensionAutomata);
        }
      }
    }
  }
#endif

     */
  os_ << "Parsed SystemC model from systemc-clang\n";
  os_ << "============= MODEL ============================\n";
  systemcModel_->dump(os_);
  os_ << "==============END========================\n";
  return true;
}

void SystemCConsumer::HandleTranslationUnit(ASTContext &context) {
  // ///////////////////////////////////////////////////////////////
  // / Pass 1: Find the necessary information.
  // ///////////////////////////////////////////////////////////////

  bool pre = false;
  pre = preFire();

  if (!pre) {
    return;
  }

  bool f = false;
  f = fire();

  if (!f) {
    return;
  }
  postFire();
}

SystemCConsumer::SystemCConsumer(ASTUnit *from_ast, std::string top)
    : os_{llvm::errs()},
      sm_{from_ast->getSourceManager()},
      context_{from_ast->getASTContext()},
      top_{top},
      systemcModel_{nullptr} {}

SystemCConsumer::SystemCConsumer(CompilerInstance &ci, std::string top)
    : os_{llvm::errs()},
      sm_{ci.getSourceManager()},
      context_{ci.getASTContext()},
      top_{top},
      systemcModel_{nullptr} {}

SystemCConsumer::~SystemCConsumer() {
  if (systemcModel_ != nullptr) {
    delete systemcModel_;
    systemcModel_ = nullptr;
  }
}

void SystemCConsumer::setTopModule(const std::string &top_module_decl) {
  top_ = top_module_decl;
}

Model *SystemCConsumer::getSystemCModel() { return systemcModel_; }

const std::string &SystemCConsumer::getTopModule() const { return top_; }

ASTContext &SystemCConsumer::getContext() const { return context_; }

SourceManager &SystemCConsumer::getSourceManager() const { return sm_; }
