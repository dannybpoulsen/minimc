#include "model/cfg.hpp"
#include "model/instructions.hpp"

#include "support/localisation.hpp"

namespace MiniMC {
  namespace Model {
    Function_ptr createEntryPoint(Program& program, Function_ptr function,std::vector<MiniMC::Model::Value_ptr>&&) {
      auto source_loc = std::make_shared<MiniMC::Model::SourceInfo>();

      static std::size_t nb = 0;
      const std::string name = MiniMC::Support::Localiser("__minimc__entry_%1%-%2%").format(function->getName(), ++nb);
      MiniMC::Model::CFA cfg;
      MiniMC::Model::RegisterDescr vstack{name};
      MiniMC::Model::LocationInfoCreator locinf (function->getName(),&vstack);
      auto funcpointer = program.getConstantFactory()->makeFunctionPointer(function->getID());
      auto init = cfg.makeLocation(locinf.make("init", 0, *source_loc));
      auto end = cfg.makeLocation(locinf.make("end", 0, *source_loc));

      cfg.setInitial(init);
      auto edge = cfg.makeEdge(init, end);

      Value_ptr result = nullptr;
      std::vector<Value_ptr> params;
      auto restype = function->getReturnType();
      if (restype->getTypeID() != TypeID::Void) {
        result = vstack.addRegister("_", restype);
      }
      edge->setAttribute<AttributeType::Instructions>(InstructionStream({createInstruction<InstructionCode::Call> ( {
		.res = result,
		.function = funcpointer,
		.params = params
	      })}
	  )
	);
	  
      
      return program.addFunction(name, {},
				 program.getTypeFactory()->makeVoidType(),
				 std::move(vstack),
				 std::move(cfg)); 
    }
  } // namespace Model
} // namespace MiniMC
