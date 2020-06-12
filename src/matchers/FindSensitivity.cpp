#include "FindSensitivity.h"
#include "FindTemplateTypes.h"
using namespace scpar;

FindSensitivity::FindSensitivity(Stmt *s, llvm::raw_ostream &os)
    : os_{os}, found_sensitive_node_{false}, clk_edge_{""} {
  /// Pass 1
  /// Find the ASTNode CXXOperatorCallExpr
  TraverseStmt(s);
}

bool FindSensitivity::shouldVisitTemplateInstantiations() const { return true; }

bool FindSensitivity::VisitMemberExpr(MemberExpr *e) {
  QualType q{e->getType()};

  if (!found_sensitive_node_) {
    string memberName = e->getMemberDecl()->getNameAsString();

    if ((q.getAsString() != "class sc_core::sc_sensitive") ||
        (memberName != "sensitive")) {
      return true;
    }
  }
  /// This is an sensitivity declaration.
  found_sensitive_node_ = true;

  // Save if it is neg or pos edge.
  auto edge{e->getMemberDecl()->getNameAsString()};
  if (edge == "pos") {
    clk_edge_ = "pos";
  } else if (edge == "neg") {
    clk_edge_ = "neg";
  }

  // os_ << "####: VisitMemberExpr -- " << clk_edge_ << ", " <<
  // e->getMemberDecl()->getNameAsString() << "\n";

  /// Check if this expression's got a type of port.
  QualType memberType{e->getMemberDecl()->getType()};
  FindTemplateTypes tt;

  tt.Enumerate(memberType.getTypePtr());

  auto type_tree{tt.getTemplateArgTreePtr()};
  auto type_str{type_tree->dft()};

  if ((type_str.find("sc_in") != std::string::npos) ||
      (type_str.find("sc_inout") != std::string::npos) ||
      (type_str.find("sc_out") != std::string::npos)) {
    return true;
  }

  sensitive_ports_.insert(
      kvType(e->getMemberDecl()->getNameAsString(), make_tuple(clk_edge_, e)));

  // Resent the private member for other clock declarations.
  clk_edge_ = "";
  return true;
}

void FindSensitivity::dump() {
  os_ << "\n ==================== Find Sensitivity ===================";
  os_ << "\n:> Print Sensitive Ports";
  for (senseMapType::iterator mit{sensitive_ports_.begin()},
       mitend{sensitive_ports_.end()};
       mit != mitend; mit++) {
    os_ << "\n:> name: " << mit->first << ", edge: " << get<0>(mit->second)
        << ", MemberExpr*: " << get<1>(mit->second);
  }
  os_ << "\n ==================== END Find Sensitivity ===================";
}

FindSensitivity::senseMapType FindSensitivity::getSenseMap() {
  return sensitive_ports_;
}

FindSensitivity::~FindSensitivity() {}