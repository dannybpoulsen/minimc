#include "support/pointer.hpp"
#include "model/variables.hpp"

namespace MiniMC {
  namespace Model {
    Variable_ptr VariableStackDescr::addVariable (const std::string& name, const Type_ptr& type) {
      variables.push_back (std::make_shared<Variable> (name));
      variables.back()->setType(type);
      variables.back()->setOwner (this->shared_from_this());
      variables.back()->setPlace ( totalSize);
      variables.back()->setId (variables.size()-1);
      totalSize += type->getSize();
      return variables.back ();
    }

    const Value_ptr ConstantFactory64::makeIntegerConstant (MiniMC::uint64_t val) {
      return Value_ptr(new MiniMC::Model::IntegerConstant (val));
    }
    
    const Value_ptr ConstantFactory64::makeLocationPointer (MiniMC::func_t id, MiniMC::offset_t block) {
      auto pptr = MiniMC::Support::makeLocationPointer (id,block);
      return Value_ptr(new MiniMC::Model::IntegerConstant (MiniMC::Support::CastPtr<MiniMC::uint64_t> (pptr)));
    }


    const Value_ptr ConstantFactory64::makeFunctionPointer (MiniMC::func_t id) {
      	auto pptr = MiniMC::Support::makeFunctionPointer (id);
	return Value_ptr(new  MiniMC::Model::IntegerConstant (MiniMC::Support::CastPtr<MiniMC::uint64_t> (pptr)));
    }
    
    const Value_ptr ConstantFactory64::makeAggregateConstant (const ConstantFactory::aggr_input& inp,bool isArr) {
      return Value_ptr(new MiniMC::Model::AggregateConstant (inp,isArr));
    }
     
    
  }
}
