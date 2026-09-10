#include "minimc/model/variables.hpp"
#include "minimc/model/valuevisitor.hpp"
#include "minimc/support/overload.hpp"
#include "minimc/support/exceptions.hpp"

#include <memory>
#include <iostream>

namespace MiniMC {
  namespace Model {
    Register_ptr RegisterDescr::addRegister(Symbol name, const Type_ptr& type) {
      auto reg = std::make_shared<Register>(name.getName(),RegisterInfo{_internal->variable_map.size(), type, _internal->types});
      reg->setType(type);
      _internal->variable_map[name.getName()] = reg;
      name.setUserData (reg);
      
      
      return reg;
    }

        
    Constant_ptr AggregateConstantBuilder::build () {
      std::size_t size = 0;
      for (auto& v : constants) {
        size += v->getType()->getSize();
      }
      std::unique_ptr<MiniMC::BV8[]> data(new MiniMC::BV8[size]);
      auto out = data.get();
      for (auto& v : constants) {
        assert(v->isConstant());
        auto addType = [&out](auto& value) {
          auto value_insert = value.getValue();
          out = std::copy(reinterpret_cast<MiniMC::BV8*>(&value_insert), reinterpret_cast<MiniMC::BV8*>(&value_insert) + sizeof(value_insert), out);
        };

	MiniMC::Model::visitValue(
					MiniMC::Support::Overload {
					  [addType](const MiniMC::Model::I8Integer& c) {addType(c);},
					    [addType](const MiniMC::Model::I16Integer& c) {addType(c);},
					    [addType](const MiniMC::Model::I32Integer& c) {addType(c);},
					    [addType](const MiniMC::Model::I64Integer& c) {addType(c);},
					    [addType](const MiniMC::Model::Bool& c) {addType(c);},
					    [addType](const MiniMC::Model::Pointer& c) {addType(c);},
					    [addType](const MiniMC::Model::Pointer32& c) {addType(c);},
					    [&out](const MiniMC::Model::AggregateConstant& aggr) {
					      auto span = aggr.getData().get_direct_access ();
					      out = std::copy(span.begin(), span.end(), out);
					    },
					    []([[maybe_unused]]const auto& l) {
					      std::cerr << l << std::endl;
					      throw MiniMC::Support::Exception("Unknown how to convert to aggregate");
					    }
					    },
					*v);
      }
      
      Type_ptr type = MiniMC::Model::AggregateType::get(size);
      
      Value_ptr v = MiniMC::Model::AggregateConstant::make (MiniMC::Util::Array{size,std::move(data)});
      v->setType (type);
      return static_pointer_cast<Constant> (v);
    }
    

    Undef::Undef(MiniMC::Model::Type_ptr t) : Constant(ValueInfo::type_t<Undef>()) {setType(t);}
    Register::Register(const std::string& name,RegisterInfo&& place) : Value(ValueInfo::type_t<Register>()),
								  place (std::move(place)),
							    
								  name(name) {}

    
    AggregateConstant::AggregateConstant(MiniMC::Util::Array&& arr) :  Constant(ValueInfo::type_t<AggregateConstant>()),data(std::move(arr)) {
      setType (MiniMC::Model::AggregateType::get(data.getSize()));
    }
    

  } // namespace Model
} // namespace MiniMC
