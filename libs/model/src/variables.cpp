#include "minimc/model/variables.hpp"
#include "minimc/model/valuevisitor.hpp"
#include "minimc/support/overload.hpp"
#include "minimc/support/exceptions.hpp"

#include <memory>
#include <iostream>

namespace MiniMC {
  namespace Model {
    Register_ptr RegisterDescr::addRegister(Symbol&& name, const Type_ptr& type) {
      auto reg = std::make_shared<Register>(std::move(name),RegisterInfo{_internal->variable_map.size(), _internal->types});
      reg->setType(type);      _internal->variable_map[reg->getSymbol()] = reg;
      reg->getSymbol().setUserData (reg);
      
      
      return reg;
    }

    const Value_ptr ConstantFactory64::makeIntegerConstant(MiniMC::BV64 val, TypeID ty) {
      Value_ptr retval;
      Type_ptr type;
      switch (ty) {
        case MiniMC::Model::TypeID::Bool:
          retval = Bool::make(val);//std::make_shared<Bool>(static_cast<MiniMC::BV8>(val));
	  type = MiniMC::Model::BoolType::get();//typefact->makeBoolType ();
	  break;
        case MiniMC::Model::TypeID::I8:
          retval = I8Integer::make(val);//std::make_shared<MiniMC::Model::TConstant<MiniMC::BV8>>(static_cast<MiniMC::BV8>(val));
	  type = MiniMC::Model::I8Type::get();//typefact->makeIntegerType (8);
	  break;
        case MiniMC::Model::TypeID::I16:
          retval = I16Integer::make(val);//std::make_shared<MiniMC::Model::TConstant<MiniMC::BV16>>(static_cast<MiniMC::BV16>(val));
	  type = MiniMC::Model::I16Type::get();//typefact->makeIntegerType (16);
	  break;
        case MiniMC::Model::TypeID::I32:
          retval = I32Integer::make(val);//std::make_shared<MiniMC::Model::TConstant<MiniMC::BV32>>(static_cast<MiniMC::BV32>(val));
	  type = MiniMC::Model::I32Type::get();//typefact->makeIntegerType (32);
	  break;
        case MiniMC::Model::TypeID::I64:
          retval = I64Integer::make(val);//std::make_shared<MiniMC::Model::TConstant<MiniMC::BV64>>(static_cast<MiniMC::BV64>(val));
	  type = MiniMC::Model::I64Type::get();//typefact->makeIntegerType (64);
	  break;
      default:
	throw MiniMC::Support::Exception("Error");
      }
      return retval;
    }


    const Value_ptr ConstantFactory64::makeFunctionPointer(MiniMC::Model::func_t id) {
      auto ptrtype = MiniMC::Model::PointerType::get();//typefact->makePointerType ();
      Value_ptr v;
      
      if (ptrtype->getSize () == 4) {
	v = Pointer32::make (MiniMC::Model::pointer32_t::makeFunctionPointer (id));//std::make_shared<MiniMC::Model::Pointer32> (MiniMC::Model::pointer32_t::makeFunctionPointer (id));  
      }

      else {
	v = Pointer::make (MiniMC::Model::pointer64_t::makeFunctionPointer (id));//std::make_shared<MiniMC::Model::Pointer> (MiniMC::Model::pointer64_t::makeFunctionPointer (id));  
      }
      
      
      v->setType (ptrtype);
      return v;
    }

    const Value_ptr ConstantFactory64::makeSymbolicConstant(const MiniMC::Model::Symbol& s) {
      return std::make_shared<SymbolicConstant> (s);
    }
      
    
    const Value_ptr ConstantFactory64::makeLocationPointer(MiniMC::Model::func_t id,MiniMC::Model::base_t lid) {
      auto ptrtype = MiniMC::Model::PointerType::get();//typefact->makePointerType ();
      Value_ptr v;

      if (ptrtype->getSize () == 4) {
	v = Pointer32::make(MiniMC::Model::pointer32_t::makeLocationPointer (id,lid));  
      }

      else {
	v = Pointer::make(MiniMC::Model::pointer64_t::makeLocationPointer (id,lid));  
      }
      

      v->setType (ptrtype);
      return v;
    }
    
    
    
    const Value_ptr ConstantFactory64::makeHeapPointer(MiniMC::Model::base_t base,MiniMC::Model::offset_t offset) {
      auto ptrtype = MiniMC::Model::PointerType::get();
      Value_ptr v;
      
      if (ptrtype->getSize () == 4) {
	v = Pointer32::make(MiniMC::Model::pointer32_t::makeHeapPointer (base,offset));  
      }

      else {
	v = Pointer::make (MiniMC::Model::pointer64_t::makeHeapPointer (base,offset));  
      }
      v->setType (ptrtype);
      return v;
    }

    const Value_ptr ConstantFactory64::makeNullPointer() {
      auto ptrtype = MiniMC::Model::PointerType::get();//typefact->makePointerType ();
      Value_ptr v;
      
      if (ptrtype->getSize () == 4) {
	v =  MiniMC::Model::Pointer::make (MiniMC::Model::pointer64_t::makeNullPointer ());	
      }
      
      else {
	v = MiniMC::Model::Pointer::make (MiniMC::Model::pointer64_t::makeNullPointer ());	
      
      }
      v->setType(ptrtype);
      return v;
    }
    
    const Value_ptr ConstantFactory64::makeUndef(TypeID ty,std::size_t size) {
      Type_ptr type;
      
      switch (ty) {
      case TypeID::I8:
	type = MiniMC::Model::I8Type::get();//typefact->makeIntegerType (8);
	break;
      case TypeID::I16:
	type = MiniMC::Model::I16Type::get();//typefact->makeIntegerType (16);
	break;
      case TypeID::I32:
	type =MiniMC::Model::I32Type::get();//type = typefact->makeIntegerType (32);
	break;
      case TypeID::I64:
	type = MiniMC::Model::I64Type::get();
	//type = typefact->makeIntegerType (64);
	break;
      case TypeID::Bool:
	type = MiniMC::Model::BoolType::get();//type = typefact->makeBoolType ();
	break;
      case TypeID::Pointer:
	type = MiniMC::Model::PointerType::get();
	//type = typefact->makePointerType ();
	break;
      case TypeID::Aggregate:
	type = MiniMC::Model::AggregateType::get(size);//typefact->makeAggregateType (size);
	break;
      default:
	throw MiniMC::Support::Exception ("Errror");
      }
      

      return  std::make_shared<MiniMC::Model::Undef>(type);
      
    }

    const Value_ptr ConstantFactory64::makeAggregateConstant(const ConstantFactory::aggr_input& inp) {
      std::size_t size = 0;
      for (auto& v : inp) {
        size += v->getType()->getSize();
      }
      assert(size);
      std::unique_ptr<MiniMC::BV8[]> data(new MiniMC::BV8[size]);
      auto out = data.get();
      for (auto& v : inp) {
        assert(v->isConstant());
        auto addType = [&out](auto& value) {
          auto value_insert = value.getValue();
          out = std::copy(reinterpret_cast<MiniMC::BV8*>(&value_insert), reinterpret_cast<MiniMC::BV8*>(&value_insert) + sizeof(value_insert), out);
        };

	MiniMC::Model::visitValue<void>(
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
      
      Value_ptr v = std::make_shared<MiniMC::Model::AggregateConstant>(MiniMC::Util::Array{size,std::move(data)});
      v->setType (type);
      return v;
  }

    Undef::Undef(MiniMC::Model::Type_ptr t) : Constant(ValueInfo<Undef>::type_t()) {setType(t);}
    Register::Register(const Symbol& name,RegisterInfo&& place) : Value(ValueInfo<Register>::type_t()),
								  place (std::move(place)),
							    
								  name(name) {}

    
    AggregateConstant::AggregateConstant(MiniMC::Util::Array&& arr) :  Constant(ValueInfo<AggregateConstant>::type_t()),data(std::move(arr)) {
      setType (MiniMC::Model::AggregateType::get(data.getSize()));
    }
    

  } // namespace Model
} // namespace MiniMC
