#include <regex>
#include <tuple>
#include "SystemCClang.h"
#include "PortBinding.h"
#include "Tree.h"
#include "Xlat.h"
#include "TemplateParametersMatcher.h"
#include "SensitivityMatcher.h"
#include "clang/Basic/FileManager.h"

using namespace std;
using namespace hnode;
using namespace scpar;

bool Xlat::postFire() {
  Model *model = getSystemCModel();
  Model::moduleMapType modules = model->getModuleDecl();

  std::error_code ec;
  string outputfn;

  FileID fileID = getSourceManager().getMainFileID();
  const FileEntry *fileentry = getSourceManager().getFileEntryForID(fileID);
  if (!fileentry) {
    outputfn = "xlatout";
    os_ << "Null file entry for tranlation unit for this astcontext\n";
  } else {
    outputfn = fileentry->getName();
    regex r("\\.cpp");
    outputfn = regex_replace(outputfn, r, "_hdl");

    os_ << "File name is " << outputfn << "\n";
  }

  llvm::raw_fd_ostream xlatout(outputfn + ".txt", ec,
                               llvm::sys::fs::CD_CreateAlways);
  os_ << "file " << outputfn << ".txt, create error code is " << ec.value()
      << "\n";
  os_ << "\n SC  Xlat plugin\n";
  int module_cnt = 0;
  for (Model::moduleMapType::iterator mit = modules.begin();
       mit != modules.end(); mit++) {
    // Second is the ModuleDecl type.
    os_ << "In module iterator loop\n";
    vector<ModuleDecl *> instanceVec =
        model->getModuleInstanceMap()[mit->second];
    for (size_t i = 0; i < instanceVec.size(); i++) {
      //os_ << "\nmodule " << mit->first << "\n";
      //string modname = mit->first + "_" + to_string(module_cnt++);

      string modname = instanceVec[i]->getName() + "_" + to_string(module_cnt++);
      os_ << "\nmodule " << modname << "\n";
			   
      hNodep h_module = new hNode(modname, hNode::hdlopsEnum::hModule);


      // Ports
      hNodep h_ports = new hNode(hNode::hdlopsEnum::hPortsigvarlist);  // list of ports, signals
      xlatport(instanceVec[i]->getIPorts(), hNode::hdlopsEnum::hPortin,
               h_ports);
      xlatport(instanceVec[i]->getInputStreamPorts(), hNode::hdlopsEnum::hPortin,
               h_ports);
      xlatport(instanceVec[i]->getOPorts(), hNode::hdlopsEnum::hPortout,
               h_ports);
      xlatport(instanceVec[i]->getOutputStreamPorts(), hNode::hdlopsEnum::hPortout,
               h_ports);
      xlatport(instanceVec[i]->getIOPorts(), hNode::hdlopsEnum::hPortio,
               h_ports);

      // Signals
      xlatsig(instanceVec[i]->getSignals(), hNode::hdlopsEnum::hSigdecl,
              h_ports);

      h_module->child_list.push_back(h_ports);
      
      // Other Variables
      //xlatvars(instanceVec[i]->getOtherVars(), model,
      // h_ports);
      xlatport(instanceVec[i]->getOtherVars(), hNode::hdlopsEnum::hVardecl, h_ports);
      // submodules
      const std::vector<ModuleDecl*> &submodv = instanceVec[i]->getNestedModuleDecl();
      os_ << "submodule count is " << submodv.size() << "\n";
      for (auto& smod:submodv) {
	os_ << "submodule " << smod->getInstanceName() << "\n";
      }

      // look at constructor

      //os_ << "dumping module constructor stmt\n";
      //instanceVec[i]->getConstructorStmt()->dump(os_);	     
      //os_ << "dumping module constructor decl\n";							       
      //instanceVec[i]->getConstructorDecl()->dump(os_);
      // Processes
      h_top = new hNode(hNode::hdlopsEnum::hProcesses);

      xlatproc(instanceVec[i]->getEntryFunctionContainer(), h_top, os_);

      if (!h_top->child_list.empty()) h_module->child_list.push_back(h_top);

      // Port bindings
      hNodep h_submodule_pb = new hNode(hNode::hdlopsEnum::hPortbindings);
      xlatportbindings(instanceVec[i]->getPortBindings(), h_submodule_pb);
      
      if (!h_submodule_pb->child_list.empty())
	h_module->child_list.push_back(h_submodule_pb);
      
      h_module->print(xlatout);
      delete h_top; //h_module;
    }
  }
  os_ << "Global Method Map\n";
  for (auto m : allmethodecls) {
    os_ << "Method --------\n" << m.first << ":" << m.second << "\n";
    m.second->dump(os_);
    os_ << "---------\n";
  }

  os_ << "User Types Map\n";

  while (!usertypes.empty()) {
    std::unordered_map<string, QualType> usertypestmp = usertypes;
    usertypes.clear();
    for (auto t : usertypestmp) {
      os_ << "User Type --------\n" << t.first << ":" << t.second.getTypePtr() << "\n";
      t.second->dump(os_);
      os_ << "---------\n";
      addtype(t.first, t.second)->print(xlatout);
    }
  }
  return true;
}

void Xlat::xlatport(ModuleDecl::portMapType pmap, hNode::hdlopsEnum h_op,
                    hNodep &h_info) {

  for (ModuleDecl::portMapType::iterator mit = pmap.begin(); mit != pmap.end();
       mit++) {

    string objname = get<0>(*mit);


    os_ << "object name is " << objname << " and h_op is " << h_op << "\n";

    PortDecl *pd = get<1>(*mit);
    Tree<TemplateType> *template_argtp = (pd->getTemplateType())->getTemplateArgTreePtr();
    //  if type is structured, it will be flattened into multiple declarations
    // each with a unique name and Typeinfo followed by Type.
    xlattype(objname, template_argtp, h_op, h_info);  // passing the sigvarlist
    // check for initializer
    VarDecl * vard = pd->getAsVarDecl();
    if (vard && vard->hasInit()) {
      APValue *apval = vard->getEvaluatedValue();
      if (apval && apval->isInt()) {
	(h_info->child_list.back())->child_list.push_back(new hNode((apval->getInt()).toString(10),  hNode::hdlopsEnum::hVarInit));
      }
    }
  }
}

void Xlat::xlatsig(ModuleDecl::signalMapType pmap, hNode::hdlopsEnum h_op,
                   hNodep &h_info) {

  for (ModuleDecl::signalMapType::iterator mit = pmap.begin();
       mit != pmap.end(); mit++) {

    string objname = get<0>(*mit);

    os_ << "object name is " << objname << "\n";

    SignalDecl *pd = get<1>(*mit);

    Tree<TemplateType> *template_argtp = (pd->getTemplateTypes())->getTemplateArgTreePtr();
    //  if type is structured, it will be flattened into multiple declarations
    // each with a unique name and Typeinfo followed by Type.
    xlattype(objname, template_argtp, h_op, h_info);  // passing the sigvarlist
   
  }
}

// Not currently used
void Xlat::makehpsv(string prefix, string typname, hNode::hdlopsEnum h_op, hNodep &h_info, bool needtypeinfo) {
  // create hNode for a Port|Signal|Var declaration with resolved type name
  os_ << "in makehpsv prefix, type, op " << prefix << " " << typname << " " << h_op << "\n";
  hNodep hmainp = new hNode(prefix, h_op);
  h_info->child_list.push_back(hmainp); // returning [opPort|Sig|Var prefix [Typeinfo [Type typname]]]

  hNodep htypep = new hNode(typname, hNode::hdlopsEnum::hType);
  if (needtypeinfo) {
    hNodep h_typeinfop = new hNode(hNode::hdlopsEnum::hTypeinfo);
    h_typeinfop->child_list.push_back(htypep);
    hmainp->child_list.push_back(h_typeinfop);
  }
  else {
    hmainp->child_list.push_back(htypep);
  }
}

void Xlat::addfieldtype(const FieldDecl * fld, hNodep &h_typdef) {
  os_ << "field of record type \n";
  fld ->dump(os_);
  os_ << "field: found name " << fld->getName() << "\n";
  // Try to get the template type of these fields.
  const Type *field_type{fld->getType().getTypePtr()};
  FindTemplateTypes find_tt{};
  find_tt.Enumerate(field_type);

  // Get the tree.
  auto template_args{find_tt.getTemplateArgTreePtr()};
  hNodep hfld = new hNode(fld->getNameAsString(), hNode::hdlopsEnum::hTypeField);
  h_typdef->child_list.push_back(hfld);
  os_ << "calling generatetype with template args of field\n";
  if (template_args->getRoot()) 
    generatetype(template_args->getRoot(), template_args, hfld);
  else os_ << "FindTemplateTypes returned null root\n";

}
hNodep Xlat::addtype(string typname, QualType qtyp) {
  hNodep h_typdef = new hNode(typname, hNode::hdlopsEnum::hTypedef);
  os_ << "addtype entered with type name " << typname << "\n";
  const Type * typ = qtyp.getTypePtr();
  if (typ->isBuiltinType())
    {
      hNodep hprim = new hNode(qtyp.getAsString(), hNode::hdlopsEnum::hType);
      os_ << "addtype found prim type " << qtyp.getAsString() << "\n";
      h_typdef->child_list.push_back(hprim);
      return h_typdef;
    }

  if (const RecordType * rectype = dyn_cast<RecordType>(typ)) {
    os_ << "addtype record type found, name is " << typname << "\n";
    if ( isa<ClassTemplateSpecializationDecl>(rectype->getDecl())) {
      os_ << "addtype isa template specialzation decl found, name is " << typname << "\n";
      ClassTemplateSpecializationDecl * ctsd = dyn_cast<ClassTemplateSpecializationDecl>(rectype->getDecl());
      ClassTemplateDecl * ctd = ctsd->getSpecializedTemplate();
      ctd->dump(os_);
      os_ << "####### ============================== MATCHER ========================= ##### \n";
      TemplateParametersMatcher template_matcher{};
      MatchFinder matchRegistry{};
      template_matcher.registerMatchers(matchRegistry);
      matchRegistry.match(*ctd, getContext());
      os_ << "####### ============================== END MATCHER ========================= ##### \n";
      

      TemplateParameterList * tpl = ctd->getTemplateParameters();
      os_ << "addtype her are template parameters\n";
      for (auto param : *tpl) {
	os_ << "addtype template param name is " << param->getName() << "\n";
	//param->dump(os_);
	h_typdef->child_list.push_back(new hNode(param->getName(), hNode::hdlopsEnum::hTypeTemplateParam));
      }
      std::vector<const FieldDecl *> fields;
      template_matcher.getFields(fields);
      if (fields.size()>0) {
	for (const FieldDecl *fld : fields) {
	  addfieldtype(fld, h_typdef);
	}
      }
    }
    else if (!rectype->getDecl()->field_empty()) {
	  for (auto const &fld: rectype->getDecl()->fields()) {
	    addfieldtype(fld, h_typdef);
	   }
	}
	else { // record type but no fields
	  // get the type name
	  os_ << "Found record with no fields, name is " << (rectype->getDecl())->getName() << "\n"; 
	  h_typdef->child_list.push_back(new hNode(rectype->getDecl()->getName(), hNode::hdlopsEnum::hType));
	      
	}
  }

  return h_typdef; 
}

void Xlat::generatetype(scpar::TreeNode<scpar::TemplateType > * const &node,
			scpar::Tree<scpar::TemplateType > * const &treehead, hNodep &h_info) {

  string tmps = (node->getDataPtr())->getTypeName();
  os_ << "generatetype node name is " << tmps << "type follows\n";
  (node->getDataPtr())->getTypePtr()->dump(os_);				 
  hNodep nodetyp = new hNode (tmps, hNode::hdlopsEnum::hType);
  h_info->child_list.push_back(nodetyp);
  if (((node->getDataPtr())->getTypePtr())->isBuiltinType())
     return;
  if (!(lutil.isSCType(tmps) || lutil.isSCBuiltinType(tmps) || lutil.isposint(tmps)))
    {
      os_ << "adding user defined type " << tmps << "\n";
      const RecordType * tstp = dyn_cast<RecordType>((node->getDataPtr())->getTypePtr());
      if (tstp) {
	//os_ << "generatetype found record type, dump of definition follows\n";
	os_ << "generatetype found record type\n";
	//RecordDecl *   tstdp = (tstp->getDecl())->getDefinition();
	//tstdp->dump(os_);
	usertypes[tmps] = ((tstp->getDecl())->getTypeForDecl())->getCanonicalTypeInternal();
      }
      usertypes[tmps] = ((node->getDataPtr())->getTypePtr())->getCanonicalTypeInternal();
    }
  auto const vectreeptr{ treehead->getChildren(node)};
  for (auto const &chnode : vectreeptr) {
    generatetype(chnode, treehead, nodetyp);
  }					       

}

void Xlat::xlattype(string prefix,  Tree<TemplateType> *template_argtp,
		     hNode::hdlopsEnum h_op, hNodep &h_info) {

  //llvm::outs()  << "xlattype dump of templatetree args follows\n";
  //template_argtp->dump();

    if (!(template_argtp &&  (template_argtp->getRoot()))) {
      os_ << "xlattype no root prefix is " << prefix << " " << template_argtp << "\n";;

    return;
    }										 
  hNodep hmainp = new hNode(prefix, h_op); // opPort|Sig|Var prefix
  h_info->child_list.push_back(hmainp);
  string tmps = ((template_argtp->getRoot())->getDataPtr())->getTypeName();
  hNodep h_typeinfo = new hNode(hNode::hdlopsEnum::hTypeinfo);
  hmainp->child_list.push_back(h_typeinfo);
  hNodep h_typ = new hNode(tmps, hNode::hdlopsEnum::hType);
  h_typeinfo->child_list.push_back(h_typ);
  
  auto const vectreeptr{ template_argtp->getChildren(template_argtp->getRoot())};
  for (auto const &node : vectreeptr) {
    generatetype(node, template_argtp, h_typ);
  }
  return;

  // Code below is not currently used. left here for reference
#if 0 
  for (auto const &node : *template_argtp) {
	const TemplateType * type_data{node->getDataPtr()}; 
	string tmps2 =  type_data->getTypeName();
	if (tmps.empty()) os_ << "xlattype builtin typename is empty\n";
	else os_ << "xlattype builtin typename is " << tmps2 << "\n";
	lutil.make_ident(tmps2);
	h_typeinfo->child_list.push_back( new hNode(tmps2, hNode::hdlopsEnum::hType));
	}

  if (template_argtp->size() == 1) {
    string tmps = ((template_argtp->getRoot())->getDataPtr())->getTypeName();  
    os_ << "primitive type " << tmps << "\n";
    lutil.make_ident(tmps);
    makehpsv(prefix, tmps, h_op, h_info);
    return;
  }

  // now process composite types
  
  
  if (template_argtp->getRoot()) {
    string tmps = ((template_argtp->getRoot())->getDataPtr())->getTypeName();
    os_ << " nonprimitive type " << tmps << "\n";
    if (lutil.isSCBuiltinType(tmps) || (lutil.isSCType(tmps)) ){ // primitive uint etc. or sc_in
      bool is_scinout = lutil.isSCType(tmps);
      hNodep hport = new hNode(prefix, h_op);
      h_info->child_list.push_back(hport);
      hNodep h_typeinfo = new hNode(hNode::hdlopsEnum::hTypeinfo);
      hport->child_list.push_back(h_typeinfo);
      if (is_scinout) {
	// sc_in, sc_out etc.
	hNodep htypep = new hNode(tmps, hNode::hdlopsEnum::hType);
	h_typeinfo->child_list.push_back(htypep); // added sc_in, etc. to type info
	auto const vectreeptr{ template_argtp->getChildren(template_argtp->getRoot())};
	for (auto const &node : vectreeptr) {
	  const Type * childtyp{(node->getDataPtr())->getTypePtr()};
	  string childns = (node->getDataPtr())->getTypeName();
	  os_ << "xlattype sctype processing child " << childns << "\n";
	  
	  const TemplateType * type_data{node->getDataPtr()}; 
	  string tmps2 =  type_data->getTypeName();
	  if (tmps2.empty()) os_ << "xlattype scinout builtin typename is empty\n";
	  else os_ << "xlattype typename is " << tmps2 << "\n";
	  if (lutil.isSCBuiltinType(tmps2)) {  // sc_in<Bool>
	    lutil.make_ident(tmps2); // but need to add parameters such as 8 in uint<8>
	    if (template_argtp->hasChildren(node)) {
	      // need to process the children of this node to generate the type parameters
	    }
	  }
	  else { // sc_in<nonprimitivetype>
	    generatetype(node, tmps2);
	  }
	  h_typeinfo->child_list.push_back( new hNode(tmps2, hNode::hdlopsEnum::hType));
	}
	  
      } // not sc_in etc, code below is for builtin type
      else for (auto const &node : *template_argtp) {
	const TemplateType * type_data{node->getDataPtr()}; 
	string tmps2 =  type_data->getTypeName();
	if (tmps.empty()) os_ << "xlattype builti typename is empty\n";
	else os_ << "xlattype builtintypename is " << tmps2 << "\n";
	lutil.make_ident(tmps2);
	h_typeinfo->child_list.push_back( new hNode(tmps2, hNode::hdlopsEnum::hType));
	}
    }

    else {
      if (const RecordType * rectype = dyn_cast<RecordType>((template_argtp->getRoot()->getDataPtr())->getTypePtr())) {
	os_ << "record type found, name is " << tmps << "\n";
	for (auto const &fld: rectype->getDecl()->fields()) {
	  os_ << "field of record type \n";
	  fld ->dump(os_);
	  os_ << "field: found name " << fld->getName() << "\n";
	  // Try to get the template type of these fields.
	  const Type *field_type{fld->getType().getTypePtr()};
	  FindTemplateTypes find_tt{};
	  find_tt.Enumerate(field_type);

	  // Get the tree.
	  auto template_args{find_tt.getTemplateArgTreePtr()};
	  // Access the tree here in the way one wishes.
	  std::string dft_str{template_args->dft()};
	  llvm::outs() << "DFT: " << dft_str << "\n";
	  xlattype(prefix+'_'+fld->getNameAsString(), template_args, h_op, h_info);
        }
      }
    }
  }
#endif
}

// Note -- not currently called
void Xlat::xlatvars(ModuleDecl::portMapType pmap, Model * model,  hNodep &h_info) {
  hNode::hdlopsEnum h_op = hNode::hdlopsEnum::hVardecl;
  for (ModuleDecl::portMapType::iterator mit = pmap.begin(); mit != pmap.end();
       mit++) {

    string objname = get<0>(*mit);

    os_ << "variable name is " << objname << " and h_op is " << h_op << "\n";

    PortDecl *pd = get<1>(*mit);
    Tree<TemplateType> *template_argtp = (pd->getTemplateType())->getTemplateArgTreePtr();
    xlattype(objname, template_argtp, h_op, h_info);  // passing the sigvarlist
    return;
    auto vmoddecl = model->getInstance(objname);
    if (vmoddecl == nullptr) {
      os_ << "variable is not a module\n";
      xlattype(objname, template_argtp, h_op, h_info);  // passing the sigvarlist
    }
    else {
      os_ << "variable is a module\n";
      string tmps;
      if (template_argtp->getRoot()) {
	tmps = ((template_argtp->getRoot())->getDataPtr())->getTypeName();
      }
      else {
	tmps = "MODULENOTYPE";
      }
      // from xlattype, builtin type processing code
      hNodep hport = new hNode(objname, h_op);
      h_info->child_list.push_back(hport);
      hNodep h_typeinfo = new hNode(hNode::hdlopsEnum::hTypeinfo);
      hport->child_list.push_back(h_typeinfo);
      for (auto const &node : *template_argtp) {
	const TemplateType * type_data{node->getDataPtr()}; 
	string tmps2 =  type_data->getTypeName();
	if (tmps.empty()) os_ << "submodule processing typename is empty\n";
	else os_ << "submodule processing typename is " << tmps2 << "\n";
	lutil.make_ident(tmps2);
	h_typeinfo->child_list.push_back( new hNode(tmps2, hNode::hdlopsEnum::hType));
      }
    }
  }
}

void Xlat::xlatproc(scpar::vector<EntryFunctionContainer *> efv, hNodep &h_top,
                    llvm::raw_ostream &os) {
  for (auto efc : efv) {
    if (efc->getProcessType() == PROCESS_TYPE::METHOD) {
      hNodep h_process = new hNode(efc->getName(), hNode::hdlopsEnum::hProcess);
      os_ << "process " << efc->getName() << "\n";
      // Sensitivity list
      EntryFunctionContainer::SenseMapType sensmap = efc->getSenseMap();
      if (!sensmap.empty()) {
	hNodep h_senslist = new hNode(hNode::hdlopsEnum::hSenslist);
	for (auto sensmapitem : sensmap) {
	  //typedef std::map<std::string, std::vector<SensitivityTupleType>> SenseMapType;
	  //typedef std::tuple<std::string, ValueDecl *, MemberExpr *>SensitivityTupleType;

	  if (sensmapitem.first == "dont_initialize") // nonsynthesizable
	    continue;
	  
	  os_ << "sensmap item " << sensmapitem.first << "\n";
	  std::vector<EntryFunctionContainer::SensitivityTupleType> sttv = sensmapitem.second;
	  EntryFunctionContainer::SensitivityTupleType sensitem0 = sttv[0];
	  hNodep h_firstfield = new hNode(get<1>(sensitem0)->getNameAsString(), hNode::hdlopsEnum::hSensvar);
	  for (int i = 1; i < sttv.size(); i++) {
	    h_firstfield->child_list.push_back(new hNode(get<1>(sttv[i])->getNameAsString(),
							 hNode::hdlopsEnum::hSensvar));
	  }
	  h_senslist->child_list.push_back(h_firstfield);
	}
	h_process->child_list.push_back(h_senslist);
      }
      CXXMethodDecl *emd = efc->getEntryMethod();
      hNodep h_body = new hNode(hNode::hdlopsEnum::hMethod);
      XlatMethod xmethod(emd, h_body, os_);  //, xlatout);
      os_ << "Method Map:\n";
      for (auto m : xmethod.methodecls) {
	os_ << m.first << ":" << m.second <<"\n";
	//m.second->dump(os_);
      }
      allmethodecls.insert(xmethod.methodecls.begin(), xmethod.methodecls.end());

      h_process->child_list.push_back(h_body);
      h_top->child_list.push_back(h_process);
    } else
      os_ << "process " << efc->getName() << " not SC_METHOD, skipping\n";
  }
}

void Xlat::xlatportbindings(scpar::ModuleDecl::portBindingMapType portbindingmap, hNodep &h_pbs){
  for (auto const &pb : portbindingmap) {
    string port_name{get<0>(pb)};
    PortBinding *binding{get<1>(pb)};
    os_ << "xlat port binding found " << port_name << "<==> " << binding->getBoundToName() << "\n";
    hNodep hpb = new hNode(hNode::hdlopsEnum::hPortbinding);
    hpb->child_list.push_back(new hNode(port_name, hNode::hdlopsEnum::hVarref));
    hpb->child_list.push_back(new hNode(binding->getBoundToName(), hNode::hdlopsEnum::hVarref));
    h_pbs->child_list.push_back(hpb);
  }
  

}
