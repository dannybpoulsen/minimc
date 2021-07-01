#include <gsl/pointers>
#include "model/cfg.hpp"
#include "model/instructions.hpp"

#include "support/localisation.hpp"


namespace MiniMC {
  namespace Model {
	gsl::not_null<Function_ptr> createEntryPoint (Program_ptr& program, gsl::not_null<Function_ptr> function) {
	  auto source_loc = std::make_shared<MiniMC::Model::SourceInfo> ();
	  
	  static std::size_t nb = 0;
	  const std::string name = MiniMC::Support::Localiser ("__minimc__entry_%1%-%2%").format (function->getName (),++nb);
	  auto cfg = program->makeCFG ();
	  auto vstack = program->makeVariableStack (name);
	  auto funcpointer = program->getConstantFactory ()->makeFunctionPointer (function->getID ());
	  auto init= cfg->makeLocation (MiniMC::Model::LocationInfo("init",0,*source_loc));
	  auto end = cfg->makeLocation (MiniMC::Model::LocationInfo("end",0,*source_loc));

	  cfg->setInitial (init);
	  auto edge = cfg->makeEdge (init,end);

	  InstBuilder<InstructionCode::Call> builder;
	  auto restype = function->getReturnType ();
	  if (restype->getTypeID () != TypeID::Void) {
		auto resvar = vstack->addVariable ("_",restype);
		builder.setRes (resvar);
	  }
	  builder.setNbParamters (program->getConstantFactory()->makeIntegerConstant (0,program->getTypeFactory ()->makeIntegerType (64)));
	  builder.setFunctionPtr (funcpointer);

	  edge->setAttribute<AttributeType::Instructions> (InstructionStream({builder.BuildInstruction ()}));
	  
	  return program->addFunction (name,{},
								   program->getTypeFactory ()->makeVoidType (),
								   vstack,
								   cfg
								   );
							  
	  
	}
  }
}
