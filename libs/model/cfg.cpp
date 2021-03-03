#include <gsl/pointers>
#include "model/cfg.hpp"
#include "model/instructions.hpp"

#include "support/localisation.hpp"


namespace MiniMC {
  namespace Model {
	gsl::not_null<Function_ptr> createEntryPoint (Program_ptr& program, gsl::not_null<Function_ptr> function) {
	  static std::size_t nb = 0;
	  auto cfg = program->makeCFG ();
	  auto vstack = program->makeVariableStack ();
	  auto funcpointer = program->getConstantFactory ()->makeFunctionPointer (function->getID ());
	  auto init= cfg->makeLocation ({"init"});
	  auto end = cfg->makeLocation ({"end"});

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
	  
	  return program->addFunction (MiniMC::Support::Localiser ("__minimc__entry_%1%-%2%").format (function->getName (),++nb),{},
								   program->getTypeFactory ()->makeVoidType (),
								   vstack,
								   cfg
								   );
							  
	  
	}
  }
}
