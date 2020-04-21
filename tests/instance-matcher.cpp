#include "SystemCClang.h"
#include "catch.hpp"
#include "clang/Tooling/Tooling.h"

#include "Matchers.h"

// This is automatically generated from cmake.
#include "ClangArgs.h"
#include "Testing.h"

using namespace clang::ast_matchers;
using namespace scpar;
using namespace sc_ast_matchers;

template <typename T>
bool find_name(std::vector<T> &names,
               const T &find_name) {
  auto found_it = std::find(names.begin(), names.end(), find_name);
  if (found_it != names.end()) {
    names.erase(found_it);
    return true;
  }

  return false;
}

TEST_CASE("Read SystemC model from file for testing", "[parsing]") {
  std::string code{systemc_clang::read_systemc_file(
      systemc_clang::test_data_dir, "xor-hierarchy.cpp")};

  ASTUnit *from_ast =
      tooling::buildASTFromCodeWithArgs(code, systemc_clang::catch_test_args)
          .release();

  llvm::outs() << "================ TESTMATCHER =============== \n";
  InstanceMatcher inst_matcher{};
  MatchFinder matchRegistry{};
  inst_matcher.registerMatchers(matchRegistry);
  // Run all the matchers
  matchRegistry.matchAST(from_ast->getASTContext());
  inst_matcher.dump();
  llvm::outs() << "================ END =============== \n";

  SECTION("Test instance matcher", "[instance-matcher]") {
    // There should be five instances here.
    // DUT2, n1, n2, n3, n4
    auto instances{inst_matcher.getInstanceMap()};

    REQUIRE(instances.size() == 5);

    std::vector<std::string> var_names{"DUT", "n1", "n2", "n3", "n4"};
    std::vector<std::string> var_type_names{"exor2", "nand2", "nand2", "nand2",
                                            "nand2"};
    std::vector<std::string> instance_names{"exor2", "N1", "N2", "N3", "N4"};

    for (auto const &entry : instances) {
      auto inst{entry.second};
      inst.dump();

      find_name(var_names, inst.var_name);
      find_name(var_type_names, inst.var_type_name);
      find_name(instance_names, inst.instance_name);

      // auto found_it =
          // std::find(var_names.begin(), var_names.end(), inst.var_name);
      // if (found_it != var_names.end()) {
        // var_names.erase(found_it);
      // }
    }
    // All the variable name should be found
    REQUIRE(var_names.size() == 0);
    REQUIRE(var_type_names.size() == 0);
    REQUIRE(instance_names.size() == 0);

    // The module instances have all the information.
  }
}
