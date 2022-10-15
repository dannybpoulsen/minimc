#include "model/variables.hpp"
#include "support/exceptions.hpp"
#include "support/pointer.hpp"

#include <memory>

namespace MiniMC {
  namespace Model {
    Register_ptr RegisterDescr::addRegister(Symbol&& name, const Type_ptr& type) {
      variables.push_back(std::make_shared<Register>(Symbol(pref, std::move(name)), this));
      variables.back()->setType(type);
      variables.back()->setPlace(totalSize);
      variables.back()->setId(variables.size() - 1);
      totalSize += type->getSize();
      return variables.back();
    }

    const Value_ptr ConstantFactory64::makeIntegerConstant(MiniMC::BV64 val, TypeID ty) {
      Value_ptr retval;
      Type_ptr type;
      switch (ty) {
        case MiniMC::Model::TypeID::Bool:
          retval.reset(new Bool(static_cast<MiniMC::BV8>(val)));
	  type = typefact->makeBoolType ();
	  break;
        case MiniMC::Model::TypeID::I8:
          retval.reset(new MiniMC::Model::TConstant<MiniMC::BV8>(static_cast<MiniMC::BV8>(val)));
	  type = typefact->makeIntegerType (8);
	  break;
        case MiniMC::Model::TypeID::I16:
          retval.reset(new MiniMC::Model::TConstant<MiniMC::BV16>(static_cast<MiniMC::BV16>(val)));
	  type = typefact->makeIntegerType (16);
	  break;
        case MiniMC::Model::TypeID::I32:
          retval.reset(new MiniMC::Model::TConstant<MiniMC::BV32>(static_cast<MiniMC::BV32>(val)));
	  type = typefact->makeIntegerType (32);
	  break;
        case MiniMC::Model::TypeID::I64:
          retval.reset(new MiniMC::Model::TConstant<MiniMC::BV64>(static_cast<MiniMC::BV64>(val)));
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
	v.reset (new MiniMC::Model::Pointer32 (MiniMC::pointer32_t::makeFunctionPointer (id)));  
      }

      else {
	v.reset (new MiniMC::Model::Pointer (MiniMC::pointer64_t::makeFunctionPointer (id)));  
      }
      
      
      v->setType (ptrtype);
      return v;
    }

    const Value_ptr ConstantFactory64::makeLocationPointer(MiniMC::func_t id,MiniMC::base_t lid) {
      auto ptrtype = typefact->makePointerType ();
      Value_ptr v;

      if (ptrtype->getSize () == 4) {
	v.reset (new MiniMC::Model::Pointer32 (MiniMC::pointer32_t::makeLocationPointer (id,lid)));  
      }

      else {
	v.reset (new MiniMC::Model::Pointer (MiniMC::pointer64_t::makeLocationPointer (id,lid)));  
      }
      

      v->setType (ptrtype);
      return v;
    }
    
    
    
    const Value_ptr ConstantFactory64::makeHeapPointer(MiniMC::base_t base) {
      auto ptrtype = typefact->makePointerType ();
      Value_ptr v;

      if (ptrtype->getSize () == 4) {
	v.reset (new MiniMC::Model::Pointer32 (MiniMC::pointer32_t::makeHeapPointer (base,0)));  
      }

      else {
	v.reset (new MiniMC::Model::Pointer (MiniMC::pointer64_t::makeHeapPointer (base,0)));  
      }
      v->setType (ptrtype);
      return v;
    }
    
    const Value_ptr ConstantFactory64::makeUndef(TypeID ty,std::size_t size) {
      Value_ptr val(new MiniMC::Model::Undef());
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
      case TypeID::Struct:
	type = typefact->makeStructType (size);
	break;
      case TypeID::Array:
	type = typefact->makeArrayType (size);
	break;
      default:
	throw MiniMC::Support::Exception ("Errror");
      }
      
      val->setType(type);
      return val;
    }

    const Value_ptr ConstantFactory64::makeAggregateConstant(const ConstantFactory::aggr_input& inp, bool isArray) {
      std::size_t size = 0;
      for (auto& v : inp) {
        size += v->getType()->getSize();
      }
      assert(size);
      std::unique_ptr<MiniMC::BV8[]> data(new MiniMC::BV8[size]);
      auto out = data.get();
      for (auto& v : inp) {
        assert(v->isConstant());
        auto c = std::static_pointer_cast<MiniMC::Model::Constant>(v);
        auto type = v->getType();
        auto addType = [&out](auto& value) {
          auto value_insert = value.getValue();
          out = std::copy(reinterpret_cast<MiniMC::BV8*>(&value_insert), reinterpret_cast<MiniMC::BV8*>(&value_insert) + sizeof(value_insert), out);
        };

        switch (type->getTypeID()) {
          case MiniMC::Model::TypeID::I8:
            addType(*std::static_pointer_cast<MiniMC::Model::I8Integer>(c));
            break;
          case MiniMC::Model::TypeID::I16:
            addType(*std::static_pointer_cast<MiniMC::Model::I16Integer>(c));
            break;
          case MiniMC::Model::TypeID::I32:
            addType(*std::static_pointer_cast<MiniMC::Model::I32Integer>(c));
            break;
          case MiniMC::Model::TypeID::I64:
            addType(*std::static_pointer_cast<MiniMC::Model::I64Integer>(c));
            break;
          case MiniMC::Model::TypeID::Bool:
            addType(*std::static_pointer_cast<MiniMC::Model::Bool>(c));
            break;
          case MiniMC::Model::TypeID::Pointer:
            addType(*std::static_pointer_cast<MiniMC::Model::Pointer>(c));
            break;
          case MiniMC::Model::TypeID::Struct:
          case MiniMC::Model::TypeID::Array: {
            auto aggr = std::static_pointer_cast<MiniMC::Model::AggregateConstant>(c);
            out = std::copy(aggr->begin(), aggr->end(), out);
	    break;
	  }
          default:
            throw MiniMC::Support::Exception("Unknown how to convert to aggregate");
        }
      }
      Type_ptr type;
      if (isArray)
	type = typefact->makeArrayType (size);
      else
	type = typefact->makeStructType (size);
      
      Value_ptr v(new MiniMC::Model::AggregateConstant(reinterpret_cast<MiniMC::BV8*>(data.get()), size));
      v->setType (type);
      return v;
  }

    Undef::Undef() : Constant(ValueInfo<Undef>::type_t()) {}
    Register::Register(const Symbol& name, RegisterDescr* owner) : Value(ValueInfo<Register>::type_t()),
                                                                        name(name), owner(owner) {}

    AggregateConstant::AggregateConstant(MiniMC::BV8* data, std::size_t s) : Constant(ValueInfo<AggregateConstant>::type_t()),
                                                                                 value(new MiniMC::BV8[s]),
                                                                                 size(s) {
      std::copy(data, data + s, value.get());
    }

  } // namespace Model
} // namespace MiniMC
