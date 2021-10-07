#include "model/variables.hpp"
#include "support/exceptions.hpp"
#include "support/pointer.hpp"

namespace MiniMC {
  namespace Model {
    Variable_ptr VariableStackDescr::addVariable(const std::string& name, const Type_ptr& type) {
      variables.push_back(std::make_shared<Variable>(pref + ":" + name));
      variables.back()->setType(type);
      variables.back()->setOwner(this->shared_from_this());
      variables.back()->setPlace(totalSize);
      variables.back()->setId(variables.size() - 1);
      totalSize += type->getSize();
      return variables.back();
    }

    const Value_ptr ConstantFactory64::makeIntegerConstant(MiniMC::uint64_t val, const Type_ptr& ty) {
      assert(ty->getTypeID() == MiniMC::Model::TypeID::Integer ||
             ty->getTypeID() == MiniMC::Model::TypeID::Bool);
      Value_ptr retval;

#define ALLOWEDTYPES  \
  X(MiniMC::uint8_t)  \
  X(MiniMC::uint16_t) \
  X(MiniMC::uint32_t) \
  X(MiniMC::uint64_t)

      switch (ty->getSize()) {
#define X(TY)                                                                   \
  case sizeof(TY):                                                              \
    retval.reset(new MiniMC::Model::IntegerConstant<TY>(static_cast<TY>(val))); \
    break;
        ALLOWEDTYPES
#undef X
#undef ALLOWEDTYPESS
        default:
          throw MiniMC::Support::Exception("HAHAH");
      }

      retval->setType(ty);
      return retval;
    }

    const Value_ptr ConstantFactory64::makeBinaryBlobConstant(MiniMC::uint8_t* val, std::size_t s) {
      return Value_ptr(new MiniMC::Model::BinaryBlobConstant(val, s));
    }

    const Value_ptr ConstantFactory64::makeLocationPointer(MiniMC::func_t id, MiniMC::offset_t block) {
      auto pptr = MiniMC::Support::makeLocationPointer(id, block);
      return Value_ptr(new MiniMC::Model::BinaryBlobConstant(reinterpret_cast<MiniMC::uint8_t*>(&pptr), sizeof(pptr)));
    }

    const Value_ptr ConstantFactory64::makeFunctionPointer(MiniMC::func_t id) {
      auto pptr = MiniMC::Support::makeFunctionPointer(id);
      return Value_ptr(new MiniMC::Model::BinaryBlobConstant(reinterpret_cast<MiniMC::uint8_t*>(&pptr), sizeof(pptr)));
    }

    const Value_ptr ConstantFactory64::makeUndef(const Type_ptr& ty) {
      Value_ptr val(new MiniMC::Model::Undef());
      val->setType(ty);
      return val;
    }

    const Value_ptr ConstantFactory64::makeAggregateConstant(const ConstantFactory::aggr_input& inp, bool isArr) {
      std::size_t size = 0;
      for (auto& v : inp) {
        size += v->getType()->getSize();
      }
      assert(size);
      std::unique_ptr<MiniMC::uint8_t[]> data(new MiniMC::uint8_t[size]);
      auto out = data.get();
      for (auto& v : inp) {
        assert(v->isConstant());
        auto c = std::static_pointer_cast<MiniMC::Model::Constant>(v);
        assert(c);
        out = std::copy(c->getData(), c->getData() + c->getType()->getSize(), out);
      }
      return Value_ptr(new MiniMC::Model::BinaryBlobConstant(reinterpret_cast<MiniMC::uint8_t*>(data.get()), size));
    }

    const Value_ptr ConstantFactory64::makeAggregateConstantNonCompile(const noncompile_aggr_input& inp, bool) {
      return Value_ptr(new MiniMC::Model::AggregateNonCompileConstant(inp));
    }

  } // namespace Model
} // namespace MiniMC
