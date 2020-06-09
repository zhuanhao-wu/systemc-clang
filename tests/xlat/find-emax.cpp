#include "catch.hpp"

#include "SystemCClang.h"

// This is automatically generated from cmake.
#include "../plugins/xlat/Xlat.h"
#include "ClangArgs.h"
#include "Testing.h"

using namespace scpar;

TEST_CASE("sreg example", "[llnl-examples]") {
  std::string code{systemc_clang::read_systemc_file(
      systemc_clang::test_data_dir, "/llnl-examples/find-emax-driver.cpp")};
  INFO(systemc_clang::test_data_dir);

  auto catch_test_args = systemc_clang::catch_test_args;
  catch_test_args.push_back("-I" + systemc_clang::test_data_dir +
                            "/llnl-examples/zfpsynth/shared");

  ASTUnit *from_ast =
      tooling::buildASTFromCodeWithArgs(code, catch_test_args).release();

  SystemCConsumer sc{from_ast};
  // Xlat sc{from_ast};
  sc.HandleTranslationUnit(from_ast->getASTContext());
  auto model{sc.getSystemCModel()};
  // These are instances.
  auto module_decl{model->getModuleDecl()};

  llvm::outs() << "\n";
  for (const auto &decl : module_decl) {
    llvm::outs() << "[ " << decl.first << "    ]" << decl.second << "   "
                 << decl.second->getInstanceName() << "\n";
  }

  // Testing instances.

  auto mymod{model->getInstance("mymodule_instance")};
  //
  // Begin the tests.
  //
  //
  SECTION("Show how to access nested sub-modules", "[instances]") {
    REQUIRE(mymod != nullptr);

    // Get the nested modules.
    auto nested_mdecls{mymod->getNestedModuleDecl()};
    llvm::outs() << "######################## NESTED SUBMODULE "
                 << nested_mdecls.size() << "\n";

    // Only u_dut should be nested in mymodule.
    REQUIRE(nested_mdecls.size() == 1);

    for (auto const &mdecl : nested_mdecls) {
      // There is only one, but showing how to access all of them, if there were
      // more than one.
      //
      REQUIRE(mdecl->getInstanceName() == "u_dut");

      // Check for port bindings
      auto port_bindings{mdecl->getPortBindings()};

      // Print all the port binding information
      for (auto const &p : port_bindings) {
        auto pb{p.second};
        pb->dump();
      }
    }
  }
}
