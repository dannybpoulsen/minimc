#include "model/variables.hpp"
#include "support/exceptions.hpp"
#include "support/pointer.hpp"

#include <memory>

namespace MiniMC {
  namespace Model {
    Register_ptr RegisterDescr::addRegister(const std::string& name, const Type_ptr& type) {
      variables.push_back(std::make_shared<Register>(pref + ":" + name, this));
      variables.back()->setType(type);
      variables.back()->setPlace(totalSize);
      variables.back()->setId(variables.size() - 1);
      totalSize += type->getSize();
      return variables.back();
    }

    const Value_ptr ConstantFactory64::makeIntegerConstant(MiniMC::BV64 val, const Type_ptr& ty) {
      assert(ty->isInteger() ||
             ty->getTypeID() == MiniMC::Model::TypeID::Bool);
      Value_ptr retval;

      switch (ty->getTypeID()) {
        case MiniMC::Model::TypeID::Bool:
          retval.reset(new Bool(static_cast<MiniMC::BV8>(val)));
          break;
        case MiniMC::Model::TypeID::I8:
          retval.reset(new MiniMC::Model::TConstant<MiniMC::BV8>(static_cast<MiniMC::BV8>(val)));
          break;
        case MiniMC::Model::TypeID::I16:
          retval.reset(new MiniMC::Model::TConstant<MiniMC::BV16>(static_cast<MiniMC::BV16>(val)));
          break;
        case MiniMC::Model::TypeID::I32:
          retval.reset(new MiniMC::Model::TConstant<MiniMC::BV32>(static_cast<MiniMC::BV32>(val)));
          break;
        case MiniMC::Model::TypeID::I64:
          retval.reset(new MiniMC::Model::TConstant<MiniMC::BV64>(static_cast<MiniMC::BV64>(val)));
          break;
        default:
          throw MiniMC::Support::Exception("Error");
      }
      retval->setType(ty);
      return retval;
    }

    const Value_ptr ConstantFactory64::makeLocationPointer(MiniMC::func_t id, MiniMC::offset_t block) {
      auto pptr = MiniMC::Support::makeLocationPointer(id, block);
      return Value_ptr(new MiniMC::Model::TConstant<pointer_t>(pptr));
    }

    const Value_ptr ConstantFactory64::makeFunctionPointer(MiniMC::func_t id) {
      auto pptr = MiniMC::Support::makeFunctionPointer(id);
      return Value_ptr(new MiniMC::Model::TConstant<pointer_t>(pptr));
    }

    const Value_ptr ConstantFactory64::makePointer(MiniMC::pointer_t pptr) {
      return Value_ptr(new MiniMC::Model::TConstant<pointer_t>(pptr));
    }

    const Value_ptr ConstantFactory64::makeUndef(const Type_ptr& ty) {
      Value_ptr val(new MiniMC::Model::Undef());
      val->setType(ty);
      return val;
    }

    const Value_ptr ConstantFactory64::makeAggregateConstant(const ConstantFactory::aggr_input& inp, bool) {
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
          }
          default:
            throw MiniMC::Support::Exception("Unknown how to convert to aggregate");
        }
      }
      return Value_ptr(new MiniMC::Model::AggregateConstant(reinterpret_cast<MiniMC::BV8*>(data.get()), size));
    }

    Undef::Undef() : Constant(ValueInfo<Undef>::type_t()) {}
    Register::Register(const std::string& name, RegisterDescr* owner) : Value(ValueInfo<Register>::type_t()),
                                                                        name(name), owner(owner) {}

    AggregateConstant::AggregateConstant(MiniMC::BV8* data, std::size_t s) : Constant(ValueInfo<AggregateConstant>::type_t()),
                                                                                 value(new MiniMC::BV8[s]),
                                                                                 size(s) {
      std::copy(data, data + s, value.get());
    }

  } // namespace Model
} // namespace MiniMC
