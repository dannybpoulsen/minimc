#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Intrinsics.h>

#include <unordered_map>
#include "support/exceptions.hpp"
#include "model/variables.hpp"
#include "model/instructions.hpp"


namespace MiniMC {
  namespace Loaders {

    using Val2ValMap = std::unordered_map<const llvm::Value*, MiniMC::Model::Value_ptr>;
    
    MiniMC::Model::Type_ptr getType (llvm::Type* type, MiniMC::Model::TypeFactory_ptr& tfactory);
    uint32_t computeSizeInBytes (llvm::Type* ty,MiniMC::Model::TypeFactory_ptr& tfactory);
    struct Types {
      MiniMC::Model::VariableStackDescr_ptr stack;
      MiniMC::Model::TypeFactory_ptr tfac;
      MiniMC::Model::Type_ptr getType (llvm::Type* type) {
		return  MiniMC::Loaders::getType (type,tfac);
      }
      std::size_t getSizeInBytes (llvm::Type* type) {
		return computeSizeInBytes (type,tfac);
      }
    };

    MiniMC::Model::Value_ptr findValue (llvm::Value* val, Val2ValMap& values, Types& tt, MiniMC::Model::ConstantFactory_ptr& );

    	
    template<unsigned>
    void translateAndAddInstruction (llvm::Instruction*, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>&, Types&, MiniMC::Model::ConstantFactory_ptr& cfac);
    
  }
}






