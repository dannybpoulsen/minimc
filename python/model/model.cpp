#include <sstream>
#include <pybind11/pybind11.h>
#include "model/types.hpp"
#include "model/variables.hpp"
#include "model/cfg.hpp"
#include "model/location.hpp"
#include "model/edge.hpp"
#include "model/instructions.hpp"

namespace py = pybind11;

void addModificationsModule (py::module& m);

void addModelModule (py::module& m) {
  auto submodule = m.def_submodule("model");
  addModificationsModule (submodule);
  
  py::class_<MiniMC::Model::Type, MiniMC::Model::Type_ptr> (submodule,"Type")
    .def ("size",&MiniMC::Model::Type::getSize);
  
  py::class_<MiniMC::Model::Value, MiniMC::Model::Value_ptr> value (submodule,"Value");
  value.def ("getType",&MiniMC::Model::Value::getType)
    .def ("__str__",&MiniMC::Model::Value::string_repr)
    .def_property ("isVariable",&MiniMC::Model::Value::isVariable,nullptr)
    .def_property ("isConstant",&MiniMC::Model::Value::isConstant,nullptr)
    .def_property ("isGlobal",&MiniMC::Model::Value::isGlobal,nullptr);
    
  py::class_<MiniMC::Model::Variable,MiniMC::Model::Variable_ptr> (submodule,"Variable",value)
    .def_property ("name",&MiniMC::Model::Variable::getName,nullptr);

  py::class_<MiniMC::Model::Constant,std::shared_ptr<MiniMC::Model::Constant> > constant (submodule,"Constant",value);
  constant
    .def_property ("isInteger",&MiniMC::Model::Constant::isInteger,nullptr)
    .def_property ("isAggregate",&MiniMC::Model::Constant::isAggregate,nullptr)
    .def_property ("isBinaryBlob",&MiniMC::Model::Constant::isBinaryBlobConstant,nullptr)
    .def_property ("isNonCompile",&MiniMC::Model::Constant::isNonCompileConstant,nullptr);
  

  py::class_<MiniMC::Model::IntegerConstant<MiniMC::uint64_t>,std::shared_ptr<MiniMC::Model::IntegerConstant<MiniMC::uint64_t>> >  (submodule,"IntConstant64",constant)
    .def_property ("int",&MiniMC::Model::IntegerConstant<MiniMC::uint64_t>::getValue,nullptr )
    .def ("__bytes__",[](std::shared_ptr<MiniMC::Model::IntegerConstant<MiniMC::uint64_t>> ptr) {
      return py::bytes ( reinterpret_cast<const char*> (ptr->getData()),ptr->getSize ());
      
    });
  
  
  py::class_<MiniMC::Model::IntegerConstant<MiniMC::uint32_t>,std::shared_ptr<MiniMC::Model::IntegerConstant<MiniMC::uint32_t>> >  (submodule,"IntConstant32",constant)
  .def_property ("int",&MiniMC::Model::IntegerConstant<MiniMC::uint32_t>::getValue,nullptr )
  .def ("__bytes__",[](std::shared_ptr<MiniMC::Model::IntegerConstant<MiniMC::uint32_t>> ptr) {
    return py::bytes ( reinterpret_cast<const char*> (ptr->getData()),ptr->getSize ());
      
    });
  
  py::class_<MiniMC::Model::IntegerConstant<MiniMC::uint16_t>,std::shared_ptr<MiniMC::Model::IntegerConstant<MiniMC::uint16_t>> >  (submodule,"IntConstant16",constant)
  .def_property ("int",&MiniMC::Model::IntegerConstant<MiniMC::uint16_t>::getValue,nullptr )
  .def ("__bytes__",[](std::shared_ptr<MiniMC::Model::IntegerConstant<MiniMC::uint16_t>> ptr) {
    return py::bytes ( reinterpret_cast<const char*> (ptr->getData()),ptr->getSize ());
      
  });
  
  py::class_<MiniMC::Model::IntegerConstant<MiniMC::uint8_t>,std::shared_ptr<MiniMC::Model::IntegerConstant<MiniMC::uint8_t>> >  (submodule,"IntConstant8",constant)
    .def_property ("int",&MiniMC::Model::IntegerConstant<MiniMC::uint8_t>::getValue,nullptr )
    .def ("__bytes__",[](std::shared_ptr<MiniMC::Model::IntegerConstant<MiniMC::uint8_t>> ptr) {
      return py::bytes ( reinterpret_cast<const char*> (ptr->getData()),ptr->getSize ());
      
  });

  py::class_<MiniMC::Model::BinaryBlobConstant,std::shared_ptr<MiniMC::Model::BinaryBlobConstant> >  (submodule,"BinaryBlob",constant)
    .def ("__bytes__",[](std::shared_ptr<MiniMC::Model::BinaryBlobConstant> ptr) {
      return py::bytes ( reinterpret_cast<const char*> (ptr->getData()),ptr->getSize ());
    });
  
  py::class_<MiniMC::Model::VariableStackDescr,MiniMC::Model::VariableStackDescr_ptr> (submodule,"VariableStack")
    .def (py::init<const std::string> ())
    .def_property ("totalVariable",&MiniMC::Model::VariableStackDescr::getTotalVariables,nullptr)
    .def_property ("totalSize",&MiniMC::Model::VariableStackDescr::getTotalSize,nullptr)
    .def ("addVariable",&MiniMC::Model::VariableStackDescr::addVariable)
    .def_property ("variables",[](const MiniMC::Model::VariableStackDescr_ptr& ptr){
      return py::make_iterator (ptr->getVariables().begin(),ptr->getVariables().end());},
      nullptr,
      py::keep_alive<0,1> ()
      );  
  
  py::class_<MiniMC::Model::TypeFactory,MiniMC::Model::TypeFactory_ptr> (submodule,"TypeFactory")
    .def ("makeIntegerType",&MiniMC::Model::TypeFactory::makeIntegerType)
    .def ("makeFloatType",&MiniMC::Model::TypeFactory::makeFloatType)
    .def ("makeBoolType",&MiniMC::Model::TypeFactory::makeBoolType)
    .def ("makeDoubleType",&MiniMC::Model::TypeFactory::makeDoubleType)
    .def ("makePointerType",&MiniMC::Model::TypeFactory::makePointerType)
    .def ("makeVoidType",&MiniMC::Model::TypeFactory::makeVoidType);

  py::class_<MiniMC::Model::ConstantFactory,MiniMC::Model::ConstantFactory_ptr> (submodule,"ConstantFactory")
    .def ("makeIntegerConstant",&MiniMC::Model::ConstantFactory::makeIntegerConstant);
    
  
  submodule
    .def ("TypeFactory64",[]() {return std::shared_ptr<MiniMC::Model::TypeFactory> (new MiniMC::Model::TypeFactory64 ());})
    .def ("ConstantFactory64",[]() {return std::shared_ptr<MiniMC::Model::ConstantFactory> (new MiniMC::Model::ConstantFactory64 ());});

  /// CFG STUFF
  py::class_<MiniMC::Model::Location,MiniMC::Model::Location_ptr> (submodule,"Location")
    .def ("addEdge",&MiniMC::Model::Location::addEdge)
    .def ("addIncomingEdge",&MiniMC::Model::Location::addIncomingEdge)
    .def ("edges", [](const MiniMC::Model::Location_ptr& ptr){
      return py::make_iterator (ptr->ebegin(),ptr->eend());},
      py::keep_alive<0,1> ()
    )
    .def  ("__str__", [](const MiniMC::Model::Location_ptr& ptr) {
      return ptr->getInfo().getName ();
    })
    .def ("__hash__",[](const MiniMC::Model::Location_ptr& ptr) {
      return reinterpret_cast<MiniMC::uint64_t> (ptr.get());
      
    });

  py::class_<MiniMC::Model::Edge,MiniMC::Model::Edge_ptr> (submodule,"Edge")
    .def_property ("from_loc",[](MiniMC::Model::Edge_ptr& ptr) {return ptr->getFrom().get();},nullptr)
    .def_property ("to_loc",[](MiniMC::Model::Edge_ptr& ptr) {return ptr->getTo().get();},&MiniMC::Model::Edge::setTo)
    .def_property ("hasInstructions",
		   &MiniMC::Model::Edge::hasAttribute<MiniMC::Model::AttributeType::Instructions>,
		   nullptr
		   )

    .def_property ("instructions",
		   [] (const MiniMC::Model::Edge_ptr& ptr) {
		     return &ptr->getAttribute<MiniMC::Model::AttributeType::Instructions> ();
		   },
		   nullptr,
		   py::return_value_policy::reference
		   )
    .def_property ("hasGuard",
		   &MiniMC::Model::Edge::hasAttribute<MiniMC::Model::AttributeType::Guard>,
		   nullptr
		   )

    .def_property ("guard",
		   [] (const MiniMC::Model::Edge_ptr& ptr) {
		     return &ptr->getAttribute<MiniMC::Model::AttributeType::Guard> ();
		   },
		   nullptr,
		   py::return_value_policy::reference
		   )
    .def ("__hash__",[](const MiniMC::Model::Edge_ptr& ptr) {
      return reinterpret_cast<MiniMC::uint64_t> (ptr.get());
      
    });
    ;

  py::class_<MiniMC::Model::CFG,MiniMC::Model::CFG_ptr> (submodule,"CFg")
    .def ("makeEdge",&MiniMC::Model::CFG::makeEdge)
    .def_property ("initialLocation",
		   [] (MiniMC::Model::CFG_ptr& ptr) {return ptr->getInitialLocation ().get();},
		   &MiniMC::Model::CFG::setInitial)
    .def ("deleteEdge", &MiniMC::Model::CFG::deleteEdge)
    .def ("deleteLocation",&MiniMC::Model::CFG::deleteLocation)
    .def ("locations",
		   [](const MiniMC::Model::CFG_ptr& ptr){
		     return py::make_iterator (ptr->getLocations ().begin(),ptr->getLocations().end());},
		   py::keep_alive<0,1> ()
      )  
    .def ("edges",
		   [](const MiniMC::Model::CFG_ptr& ptr){
		     return py::make_iterator (ptr->getEdges ().begin(),ptr->getEdges().end());},
		   py::keep_alive<0,1> ()
		   )
    ;

  py::class_<MiniMC::Model::Guard> (submodule,"Guard")
    .def_readwrite ("guard",&MiniMC::Model::Guard::guard)
    .def_readwrite ("negated",&MiniMC::Model::Guard::negate)
    .def ("__str__",[](MiniMC::Model::Guard* guard) {
      std::stringstream str;
      str << *guard;
      return str.str();
    });
  
  py::class_<MiniMC::Model::Function,MiniMC::Model::Function_ptr> (submodule,"Function")
    .def_property ("name",&MiniMC::Model::Function::getName,nullptr)
    .def_property ("variable_stack",[](MiniMC::Model::Function_ptr& ptr) {return ptr->getVariableStackDescr ();},nullptr)
    .def_property ("cfg",
		   [] (MiniMC::Model::Function_ptr& ptr) {
		     return ptr->getCFG().get();},
		   nullptr)
    .def_property ("return_type",&MiniMC::Model::Function::getReturnType,nullptr);

  py::class_<MiniMC::Model::Program,MiniMC::Model::Program_ptr> (submodule,"Program")
    .def_property ("functions",[](const MiniMC::Model::Program_ptr& ptr){
      return py::make_iterator (ptr->getFunctions ().begin(),ptr->getFunctions().end());},
      nullptr,
      py::keep_alive<0,1> ()
      )
    .def ("getFunction",[](MiniMC::Model::Program_ptr& ptr, const std::string& name) {
      return ptr->getFunction (name).get();
    })
    
    .def ("getEntryPoints",[](const MiniMC::Model::Program_ptr& ptr){
      return py::make_iterator (ptr->getEntryPoints ().begin(),ptr->getEntryPoints().end());},
      py::keep_alive<0,1> ()
      )
    .def ("makeEntryPoint",[](MiniMC::Model::Program_ptr& program,
			      MiniMC::Model::Function_ptr function) {
      
      return MiniMC::Model::createEntryPoint (program,function).get();
    })
    .def ("addEntryPoint",[](MiniMC::Model::Program_ptr& prgm,
			     const MiniMC::Model::Function_ptr& function) {
      
      prgm->addEntryPoint (function->getName ());
    })
    .def_property ("initialiser",
		   [](const MiniMC::Model::Program_ptr& ptr) {return &ptr->getInitialisation ();},
		   nullptr,
		   py::return_value_policy::reference)
    ;
  

  py::class_<MiniMC::Model::InstructionStream> (submodule,"InstructionStream")
    .def_property ("instructions",[](MiniMC::Model::InstructionStream* stream) {
      
      return py::make_iterator (stream->begin(),stream->end());},
      nullptr,py::keep_alive<0,1> ()
      ); 



  py::class_<MiniMC::Model::Instruction> instruction (submodule,"Instruction");
    
  py::enum_<MiniMC::Model::InstructionCode>(instruction, "OpCodes")
#define X(OP)						\
    .value(#OP,MiniMC::Model::InstructionCode::OP)
    OPERATIONS
    .export_values();
#undef X

  instruction
    .def ("__str__",[](MiniMC::Model::Instruction* s) {
    std::stringstream str;
    s->output (str);
    return str.str ();
  })
    .def ("opcode",&MiniMC::Model::Instruction::getOpcode)
    .def_property ("ops", [](MiniMC::Model::Instruction* instr) {
      return py::make_iterator (instr->begin(),instr->end());},
      nullptr,py::keep_alive<0,1> ()
      );
}
