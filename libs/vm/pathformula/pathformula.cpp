#include "model/valuevisitor.hpp"
#include "smt/builder.hpp"
#include "util/smtconstruction.hpp"
#include "vm/pathformula/pathformua.hpp"
#include "vm/pathformula/value.hpp"
#include <sstream>

namespace MiniMC {
  namespace VMT {
    namespace Pathformula {
      std::size_t next = 0;

      inline PathFormulaVMVal unboundVal(const MiniMC::Model::Type_ptr& t, SMTLib::TermBuilder& builder) {
        std::stringstream str;
        str << "Var" << ++next;
        switch (t->getTypeID()) {
          case MiniMC::Model::TypeID::Bool:
            return BoolValue(builder.makeVar(builder.makeSort(SMTLib::SortKind::Bool, {}), str.str()));

          case MiniMC::Model::TypeID::Pointer:
            return PointerValue(builder.makeVar(builder.makeBVSort(64), str.str()));
          case MiniMC::Model::TypeID::I8:
            return I8Value(builder.makeVar(builder.makeBVSort(8), str.str()));
          case MiniMC::Model::TypeID::I16:
            return I16Value(builder.makeVar(builder.makeBVSort(16), str.str()));
          case MiniMC::Model::TypeID::I32:
            return I32Value(builder.makeVar(builder.makeBVSort(32), str.str()));
          case MiniMC::Model::TypeID::I64:
            return I64Value(builder.makeVar(builder.makeBVSort(64), str.str()));
          case MiniMC::Model::TypeID::Array:
          case MiniMC::Model::TypeID::Struct:
            return AggregateValue(builder.makeVar(builder.makeBVSort(8 * t->getSize()), str.str()), t->getSize());

          default:
            break;
        }
        throw MiniMC::Support::Exception("Erro");
      }
      PathFormulaVMVal Memory::loadValue(const typename PathFormulaVMVal::Pointer&, const MiniMC::Model::Type_ptr& t) const {
        auto val = unboundVal(t, builder);
        return val;
      }

      PathFormulaVMVal Memory::alloca(const PathFormulaVMVal::I64&) {
        std::stringstream str;
        str << "Var" << ++next;

        return PointerValue(builder.makeVar(builder.makeBVSort(64), str.str()));
      }

      PathFormulaVMVal ValueLookup::unboundValue(const MiniMC::Model::Type_ptr& t) const {
        return unboundVal(t, builder);
      }

      PathFormulaVMVal ValueLookup::lookupValue(const MiniMC::Model::Value_ptr& v) const {
        return MiniMC::Model::visitValue(
            MiniMC::Model::Overload{
                [this](const MiniMC::Model::I8Integer& val) -> PathFormulaVMVal { return I8Value(builder.makeBVIntConst(val.getValue(), 8)); },
                [this](const MiniMC::Model::I16Integer& val) -> PathFormulaVMVal { return I16Value(builder.makeBVIntConst(val.getValue(), 16)); },
                [this](const MiniMC::Model::I32Integer& val) -> PathFormulaVMVal { return I32Value(builder.makeBVIntConst(val.getValue(), 32)); },
                [this](const MiniMC::Model::I64Integer& val) -> PathFormulaVMVal { return I64Value(builder.makeBVIntConst(val.getValue(), 64)); },
                [this](const MiniMC::Model::Bool& val) -> PathFormulaVMVal { return BoolValue(builder.makeBoolConst(val.getValue())); },
                [this](const MiniMC::Model::Pointer& val) -> PathFormulaVMVal { return PointerValue(builder.makeBVIntConst(std::bit_cast<uint64_t>(val.getValue()), 64)); },
                [this](const MiniMC::Model::AggregateConstant& val) -> PathFormulaVMVal {
                  MiniMC::Util::Chainer<SMTLib::Ops::Concat> chainer{&builder};
                  for (auto byte : val) {
                    chainer >> (builder.makeBVIntConst(byte, 8));
                  }
                  return AggregateValue(chainer.getTerm(), val.getSize());
                },
                [this](const MiniMC::Model::Undef& val) -> PathFormulaVMVal { return unboundValue(val.getType()); },
                [this](const MiniMC::Model::Register& val) -> PathFormulaVMVal {
                  return values.get(val);
                },
            },
            *v);
      }

      PathControl::PathControl(SMTLib::TermBuilder& builder) : builder(builder) {
        assump = nullptr;
      }

      TriBool PathControl::addAssumption(const PathFormulaVMVal::Bool& b) {
        if (assump)
          assump = builder.buildTerm(SMTLib::Ops::And, {assump, b.getTerm()});
        else
          assump = b.getTerm();
        return TriBool::Unk;
      }

      TriBool PathControl::addAssert(const PathFormulaVMVal::Bool&) {
        return TriBool::Unk;
      }

      template <ValType v>
      std::ostream& Value<v>::output(std::ostream& os) const {
        return term->output(os);
      }

      template class Value<ValType::Bool>;
      template class Value<ValType::Pointer>;
      template class Value<ValType::I8>;
      template class Value<ValType::I16>;
      template class Value<ValType::I32>;
      template class Value<ValType::I64>;
      template class Value<ValType::Aggregate>;

    } // namespace Pathformula
  }   // namespace VMT
} // namespace MiniMC
