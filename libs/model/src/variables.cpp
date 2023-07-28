#include "model/variables.hpp"
#include "model/valuevisitor.hpp"
#include "support/overload.hpp"
#include "support/exceptions.hpp"

#include <memory>

namespace MiniMC {
  namespace Model {
    Register_ptr RegisterDescr::addRegister(Symbol&& name, const Type_ptr& type) {
      _internal->variables.push_back(std::make_shared<Register>(std::move(name),RegisterInfo{_internal->variables.size(), _internal->types}));
      _internal->variables.back()->setType(type);
      return _internal->variables.back();
    }

    const Value_ptr ConstantFactory64::makeIntegerConstant(MiniMC::BV64 val, TypeID ty) {
      Value_ptr retval;
      Type_ptr type;
      switch (ty) {
        case MiniMC::Model::TypeID::Bool:
          retval = std::make_shared<Bool>(static_cast<MiniMC::BV8>(val));
	  type = typefact->makeBoolType ();
	  break;
        case MiniMC::Model::TypeID::I8:
          retval = std::make_shared<MiniMC::Model::TConstant<MiniMC::BV8>>(static_cast<MiniMC::BV8>(val));
	  type = typefact->makeIntegerType (8);
	  break;
        case MiniMC::Model::TypeID::I16:
          retval = std::make_shared<MiniMC::Model::TConstant<MiniMC::BV16>>(static_cast<MiniMC::BV16>(val));
	  type = typefact->makeIntegerType (16);
	  break;
        case MiniMC::Model::TypeID::I32:
          retval = std::make_shared<MiniMC::Model::TConstant<MiniMC::BV32>>(static_cast<MiniMC::BV32>(val));
	  type = typefact->makeIntegerType (32);
	  break;
        case MiniMC::Model::TypeID::I64:
          retval = std::make_shared<MiniMC::Model::TConstant<MiniMC::BV64>>(static_cast<MiniMC::BV64>(val));
	  type = typefact->makeIntegerType (64);
	  break;
      default:
	throw MiniMC::Support::Exception("Error");
      }
      retval->setType(type);
      return retval;
    }


    const Value_ptr ConstantFactory64::makeFunctionPointer(MiniMC::func_t id) {
      auto ptrtype = typefact->makePointerType ();
      Value_ptr v;
      
      if (ptrtype->getSize () == 4) {
	v = std::make_shared<MiniMC::Model::Pointer32> (MiniMC::pointer32_t::makeFunctionPointer (id));  
      }

      else {
	v = std::make_shared<MiniMC::Model::Pointer> (MiniMC::pointer64_t::makeFunctionPointer (id));  
      }
      
      
      v->setType (ptrtype);
      return v;
    }

    const Value_ptr ConstantFactory64::makeSymbolicConstant(const MiniMC::Model::Symbol& s) {
      return std::make_shared<SymbolicConstant> (s);
    }
      
    
    const Value_ptr ConstantFactory64::makeLocationPointer(MiniMC::func_t id,MiniMC::base_t lid) {
      auto ptrtype = typefact->makePointerType ();
      Value_ptr v;

      if (ptrtype->getSize () == 4) {
	v = std::make_shared<MiniMC::Model::Pointer32> (MiniMC::pointer32_t::makeLocationPointer (id,lid));  
      }

      else {
	v = std::make_shared<MiniMC::Model::Pointer> (MiniMC::pointer64_t::makeLocationPointer (id,lid));  
      }
      

      v->setType (ptrtype);
      return v;
    }
    
    
    
    const Value_ptr ConstantFactory64::makeHeapPointer(MiniMC::base_t base,MiniMC::offset_t offset) {
      auto ptrtype = typefact->makePointerType ();
      Value_ptr v;
      
      if (ptrtype->getSize () == 4) {
	v = std::make_shared<MiniMC::Model::Pointer32> (MiniMC::pointer32_t::makeHeapPointer (base,offset));  
      }

      else {
	v = std::make_shared<MiniMC::Model::Pointer> (MiniMC::pointer64_t::makeHeapPointer (base,offset));  
      }
      v->setType (ptrtype);
      return v;
    }

    const Value_ptr ConstantFactory64::makeNullPointer() {
      auto ptrtype = typefact->makePointerType ();
      Value_ptr v;
      
      if (ptrtype->getSize () == 4) {
	v =  std::make_shared<MiniMC::Model::Pointer> (MiniMC::pointer64_t::makeNullPointer ());	
      }
      
      else {
	v = std::make_shared<MiniMC::Model::Pointer> (MiniMC::pointer64_t::makeNullPointer ());	
      
      }
      v->setType(ptrtype);
      return v;
    }
    
    const Value_ptr ConstantFactory64::makeUndef(TypeID ty,std::size_t size) {
      Value_ptr val = std::make_shared<MiniMC::Model::Undef>();
      Type_ptr type;

      switch (ty) {
      case TypeID::I8:
	type = typefact->makeIntegerType (8);
	break;
      case TypeID::I16:
	type = typefact->makeIntegerType (16);
	break;
      case TypeID::I32:
	type = typefact->makeIntegerType (32);
	break;
      case TypeID::I64:
	type = typefact->makeIntegerType (64);
	break;
      case TypeID::Bool:
	type = typefact->makeBoolType ();
	break;
      case TypeID::Pointer:
	type = typefact->makePointerType ();
	break;
      case TypeID::Aggregate:
	type = typefact->makeAggregateType (size);
	break;
      default:
	throw MiniMC::Support::Exception ("Errror");
      }
      
      val->setType(type);
      return val;
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
				      out = std::copy(aggr.getData().begin(), aggr.getData().end(), out);
				    },
				    [](auto& l) {
				      std::cerr << l << std::endl; 
				      throw MiniMC::Support::Exception("Unknown how to convert to aggregate");
				    }
				      },
				  *v);
      }
      
      Type_ptr type = typefact->makeAggregateType (size);
      
      Value_ptr v = std::make_shared<MiniMC::Model::AggregateConstant>(MiniMC::Util::Array{size,std::move(data)});
      v->setType (type);
      return v;
  }

    Undef::Undef() : Constant(ValueInfo<Undef>::type_t()) {}
    Register::Register(const Symbol& name,RegisterInfo&& place) : Value(ValueInfo<Register>::type_t()),
							  place (std::move(place)),
							    
							    name(name) {}

    
    AggregateConstant::AggregateConstant(MiniMC::Util::Array&& arr) :  Constant(ValueInfo<AggregateConstant>::type_t()),data(std::move(arr)) {}
    

  } // namespace Model
} // namespace MiniMC
