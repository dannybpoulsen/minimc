#include "model/checkers/typechecker.hpp"

#include "model/cfg.hpp"
#include "support/feedback.hpp"
#include "support/localisation.hpp"
#include "support/pointer.hpp"

namespace MiniMC {
  namespace Model {
    namespace Checkers {
      template <MiniMC::Model::InstructionCode i, typename t = void>
      struct TypeCheck {
        static bool doCheck(MiniMC::Model::Instruction&, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr, MiniMC::Model::Program_ptr& prgm) {
          MiniMC::Support::Localiser no_typecheck_for("No typechecking implemented for '%1%'");
          mess.error(no_typecheck_for.format(i));

          return false;
        }
      };

      template <MiniMC::Model::InstructionCode i>
      struct TypeCheck<i, typename std::enable_if<MiniMC::Model::InstructionData<i>::isTAC>::type> {
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          MiniMC::Support::Localiser loc("All operands to '%1%' must have same type as the result.");
          InstHelper<i> h(inst);
          auto resType = h.getResult()->getType();
          auto lType = h.getLeftOp()->getType();
          auto rType = h.getRightOp()->getType();
          if (resType != lType ||
              lType != rType ||
              rType != resType) {
            mess.error(loc.format(i));
            return false;
          }
          return true;
        }
      };

      template <MiniMC::Model::InstructionCode i>
      struct TypeCheck<i, typename std::enable_if<MiniMC::Model::InstructionData<i>::isPredicate>::type> {
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          MiniMC::Support::Localiser loc("All operands to '%1%' must be same type.");
          InstHelper<i> h(inst);
          auto lType = h.getLeftOp()->getType();
          auto rType = h.getRightOp()->getType();
          if (lType != rType) {
            mess.error(loc.format(i));
            return false;
          }
          return true;
        }
      };

      template <MiniMC::Model::InstructionCode i>
      struct TypeCheck<i, typename std::enable_if<MiniMC::Model::InstructionData<i>::isUnary>::type> {
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          if constexpr (i == MiniMC::Model::InstructionCode::Not) {
            MiniMC::Support::Localiser loc("All operands to '%1%' must have same type as the result.");
            InstHelper<i> h(inst);
            auto resType = h.getResult()->getType();
            auto lType = h.getOp()->getType();
            if (resType != lType) {
              mess.error(loc.format(i));
              return false;
            }
            return true;
          }
        }
      };

      template <MiniMC::Model::InstructionCode i>
      struct TypeCheck<i, typename std::enable_if<MiniMC::Model::InstructionData<i>::isComparison>::type> {
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          MiniMC::Support::Localiser loc("All operands to '%1%' must have same type..");
          MiniMC::Support::Localiser res_must_be_bool("The result of '%1% must be boolean.");

          InstHelper<i> h(inst);
          auto resType = h.getResult()->getType();
          auto lType = h.getLeftOp()->getType();
          auto rType = h.getRightOp()->getType();
          if (lType != rType) {
            mess.error(loc.format(i));
            return false;
          } else if (resType->getTypeID() != MiniMC::Model::TypeID::Bool) {
            mess.error(res_must_be_bool.format(i));
            return false;
          }

          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::Trunc, void> {
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          MiniMC::Support::Localiser trunc_must_be_integer("'%1%' can only be applied to integer types. ");
          MiniMC::Support::Localiser trunc_must_be_larger("From type must be larger that to type for '%1%'");

          InstHelper<MiniMC::Model::InstructionCode::Trunc> h(inst);
          auto ftype = h.getCastee()->getType();
          auto ttype = h.getResult()->getType();
          if (ftype->getTypeID() != MiniMC::Model::TypeID::Integer ||
              ttype->getTypeID() != MiniMC::Model::TypeID::Integer) {
            mess.error(trunc_must_be_integer.format(MiniMC::Model::InstructionCode::Trunc));
            return false;
          } else if (ftype->getSize() <= ttype->getSize()) {
            mess.error(trunc_must_be_larger.format(MiniMC::Model::InstructionCode::Trunc));
            return false;
          }

          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::IntToBool> {
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          MiniMC::Support::Localiser must_be_integer("'%1%' can only be applied to integer types. ");
          MiniMC::Support::Localiser must_be_smaller("From type must be smaller that to type for '%1%'");

          InstHelper<MiniMC::Model::InstructionCode::IntToBool> h(inst);
          auto ftype = h.getCastee()->getType();
          auto ttype = h.getResult()->getType();
          if (ftype->getTypeID() != MiniMC::Model::TypeID::Integer ||
              ttype->getTypeID() != MiniMC::Model::TypeID::Bool) {
            mess.error(must_be_integer.format(MiniMC::Model::InstructionCode::IntToBool));
            return false;
          }
          return true;
        }
      };

      template <MiniMC::Model::InstructionCode i>
      struct TypeCheck<i, typename std::enable_if_t<
                              i == MiniMC::Model::InstructionCode::SExt ||
                              i == MiniMC::Model::InstructionCode::ZExt>> {
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          MiniMC::Support::Localiser must_be_integer("'%1%' can only be applied to integer types. ");
          MiniMC::Support::Localiser must_be_smaller("From type must be smaller that to type for '%1%'");

          InstHelper<i> h(inst);
          auto ftype = h.getCastee()->getType();
          auto ttype = h.getResult()->getType();
          if (ftype->getTypeID() != MiniMC::Model::TypeID::Integer ||
              ttype->getTypeID() != MiniMC::Model::TypeID::Integer) {
            mess.error(must_be_integer.format(i));
            return false;
          } else if (ftype->getSize() > ttype->getSize()) {
            mess.error(must_be_smaller.format(i));
            return false;
          }

          return true;
        }
      };

      template <MiniMC::Model::InstructionCode i>
      struct TypeCheck<i, typename std::enable_if<
                              i == MiniMC::Model::InstructionCode::BoolSExt ||
                              i == MiniMC::Model::InstructionCode::BoolZExt>::type> {
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          MiniMC::Support::Localiser must_be_integer("'%1%' can only be applied from boolean types to  integer types. ");
          MiniMC::Support::Localiser must_be_smaller("From type must be smaller that to type for '%1%'");

          InstHelper<i> h(inst);
          auto ftype = h.getCastee()->getType();
          auto ttype = h.getResult()->getType();
          if (ftype->getTypeID() != MiniMC::Model::TypeID::Bool ||
              ttype->getTypeID() != MiniMC::Model::TypeID::Integer) {
            mess.error(must_be_integer.format(i));
            return false;
          } else if (ftype->getSize() > ttype->getSize()) {
            mess.error(must_be_smaller.format(i));
            return false;
          }

          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::IntToPtr, void> {
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          MiniMC::Support::Localiser must_be_integer("'%1%' can only be applied to integer types. ");
          MiniMC::Support::Localiser must_be_pointer("Return type has to be pointer for '%1%'");

          InstHelper<MiniMC::Model::InstructionCode::IntToPtr> h(inst);
          auto ftype = h.getCastee()->getType();
          auto ttype = h.getResult()->getType();
          if (ftype->getTypeID() != MiniMC::Model::TypeID::Integer) {
            mess.error(must_be_integer.format(MiniMC::Model::InstructionCode::IntToPtr));
            return false;
          }

          else if (ttype->getTypeID() != MiniMC::Model::TypeID::Pointer) {
            mess.error(must_be_pointer.format(MiniMC::Model::InstructionCode::IntToPtr));
            return false;
          }

          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::PtrAdd, void> {
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          MiniMC::Support::Localiser must_be_integer("'%2%' has to be an integer for '%1%'. ");
          MiniMC::Support::Localiser must_be_same_type("'value and skipeSize must be same type '%1%'. ");

          MiniMC::Support::Localiser must_be_pointer("Return type has to be pointer for '%1%'");
          MiniMC::Support::Localiser base_must_be_pointer("Base  has to be pointer for '%1%'");

          InstHelper<MiniMC::Model::InstructionCode::PtrAdd> h(inst);
          auto ptr = h.getAddress()->getType();
          auto skip = h.getSkipSize()->getType();
          auto value = h.getValue()->getType();
          auto result = h.getResult()->getType();

          if (result->getTypeID() != MiniMC::Model::TypeID::Pointer) {
            mess.error(must_be_integer.format(MiniMC::Model::InstructionCode::PtrAdd));
            return false;
          } else if (skip->getTypeID() != MiniMC::Model::TypeID::Integer) {
            mess.error(must_be_integer.format(MiniMC::Model::InstructionCode::PtrAdd, "SkipSize"));
            return false;
          } else if (value->getTypeID() != MiniMC::Model::TypeID::Integer) {
            mess.error(must_be_integer.format(MiniMC::Model::InstructionCode::PtrAdd, "Value"));
            return false;
          } else if (value != skip) {
            mess.error(must_be_same_type.format(MiniMC::Model::InstructionCode::PtrAdd, "Value"));
            return false;
          }

          else if (ptr->getTypeID() != MiniMC::Model::TypeID::Pointer) {
            mess.error(base_must_be_pointer.format(MiniMC::Model::InstructionCode::PtrAdd));
            return false;
          }

          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::PtrToInt> {
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only be applied to pointer types. ");
          MiniMC::Support::Localiser must_be_integer("Return type has to be integer for '%1%'");

          InstHelper<MiniMC::Model::InstructionCode::PtrToInt> h(inst);
          auto ftype = h.getCastee()->getType();
          auto ttype = h.getResult()->getType();
          if (ftype->getTypeID() != MiniMC::Model::TypeID::Pointer) {
            mess.error(must_be_pointer.format(MiniMC::Model::InstructionCode::PtrToInt));
            return false;
          }

          else if (ttype->getTypeID() != MiniMC::Model::TypeID::Integer) {
            mess.error(must_be_pointer.format(MiniMC::Model::InstructionCode::PtrToInt));
            return false;
          }

          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::Alloca> {
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only return pointer types. ");
          MiniMC::Support::Localiser must_be_integer("Size parameter to '%1%' must be an Integer and 8 bytes long. ");

          InstHelper<MiniMC::Model::InstructionCode::Alloca> h(inst);
          auto restype = h.getResult()->getType();
          auto alloc = h.getResult();
          auto sizetype = h.getSize()->getType();

          if (restype->getTypeID() != MiniMC::Model::TypeID::Pointer) {
            mess.error(must_be_pointer.format(MiniMC::Model::InstructionCode::Alloca));
            return false;
          }

          if (sizetype->getTypeID() != MiniMC::Model::TypeID::Integer ||
              sizetype->getSize() != 8) {
            mess.error(must_be_integer.format(MiniMC::Model::InstructionCode::Malloc));
            return false;
          }

          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::ExtendObj> {
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only return pointer types. ");
          MiniMC::Support::Localiser must_be_integer("Size parameter to '%1%' must be an Integer and 8 bytes long. ");

          InstHelper<MiniMC::Model::InstructionCode::ExtendObj> h(inst);
          auto restype = h.getResult()->getType();
          auto pointertype = h.getPointer()->getType();
          auto sizetype = h.getSize()->getType();

          if (restype->getTypeID() != MiniMC::Model::TypeID::Pointer) {
            mess.error(must_be_pointer.format(MiniMC::Model::InstructionCode::Alloca));
            return false;
          }

          if (pointertype->getTypeID() != MiniMC::Model::TypeID::Pointer) {
            mess.error(must_be_pointer.format(MiniMC::Model::InstructionCode::ExtendObj));
            return false;
          }

          if (sizetype->getTypeID() != MiniMC::Model::TypeID::Integer ||
              sizetype->getSize() != 8) {
            mess.error(must_be_integer.format(MiniMC::Model::InstructionCode::ExtendObj));
            return false;
          }

          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::BitCast> {
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          MiniMC::Support::Localiser warning("TypeCheck not fully implemented for '%1%'");
          mess.warning(warning.format(MiniMC::Model::InstructionCode::BitCast));
          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::Malloc> {
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only accept pointer types. ");
          MiniMC::Support::Localiser must_be_integer("Size parameter to '%1%' must be an Integer and 8 bytes long. ");

          InstHelper<MiniMC::Model::InstructionCode::Malloc> h(inst);
          auto ptrtype = h.getPointer()->getType();
          auto sizetype = h.getSize()->getType();
          if (ptrtype->getTypeID() != MiniMC::Model::TypeID::Pointer) {
            mess.error(must_be_pointer.format(MiniMC::Model::InstructionCode::Malloc));
            return false;
          }

          if (sizetype->getTypeID() != MiniMC::Model::TypeID::Integer ||
              sizetype->getSize() != 8) {
            mess.error(must_be_integer.format(MiniMC::Model::InstructionCode::Malloc));
            return false;
          }

          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::Free> {
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only accept pointer types. ");

          InstHelper<MiniMC::Model::InstructionCode::Free> h(inst);
          auto ptrtype = h.getPointer()->getType();

          if (ptrtype->getTypeID() != MiniMC::Model::TypeID::Pointer) {
            mess.error(must_be_pointer.format(MiniMC::Model::InstructionCode::Malloc));
            return false;
          }

          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::FindSpace> {
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only return pointer types. ");
          MiniMC::Support::Localiser must_be_integer("Size parameter to '%1%' must be an Integer. ");

          InstHelper<MiniMC::Model::InstructionCode::FindSpace> h(inst);
          auto restype = h.getResult()->getType();
          auto sizetype = h.getSize()->getType();
          if (restype->getTypeID() != MiniMC::Model::TypeID::Pointer) {
            mess.error(must_be_pointer.format(MiniMC::Model::InstructionCode::FindSpace));
            return false;
          }

          if (sizetype->getTypeID() != MiniMC::Model::TypeID::Integer) {
            mess.error(must_be_integer.format(MiniMC::Model::InstructionCode::FindSpace));
            return false;
          }

          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::Store> {
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only store to pointer types. ");

          InstHelper<MiniMC::Model::InstructionCode::Store> h(inst);
          auto addr = h.getAddress()->getType();
          if (addr->getTypeID() != MiniMC::Model::TypeID::Pointer) {
            mess.error(must_be_pointer.format(MiniMC::Model::InstructionCode::Store));
            return false;
          }

          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::Load> {
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only load from pointer types. ");
          MiniMC::Support::Localiser must_be_integer("'%1%' can only load integers. ");

          InstHelper<MiniMC::Model::InstructionCode::Load> h(inst);
          auto addr = h.getAddress()->getType();
          if (addr->getTypeID() != MiniMC::Model::TypeID::Pointer) {
            mess.error(must_be_pointer.format(MiniMC::Model::InstructionCode::Load));
            return false;
          }

          if (h.getResult()->getType()->getTypeID() != MiniMC::Model::TypeID::Integer) {
            mess.error(must_be_integer.format(MiniMC::Model::InstructionCode::Load));
            return false;
          }

          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::Skip> {
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::Call> {
        static const MiniMC::Model::InstructionCode OpCode = MiniMC::Model::InstructionCode::Call;
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          //auto prgm = inst.getFunction()->getPrgm ();
          InstHelper<OpCode> h(inst);
          auto func = h.getFunctionPtr();
          if (!func->isConstant()) {
            MiniMC::Support::Localiser must_be_constant("'%1%' can only use constant function pointers. ");
            mess.error(must_be_constant.format(OpCode));
            return false;
          }

          else {
            auto constant = std::static_pointer_cast<MiniMC::Model::BinaryBlobConstant>(func);
            ;
            auto ptr = (*constant).template getValue<MiniMC::pointer_t>(); //MiniMC::Support::CastToPtr (constant->getValue());
            bool is_func = prgm->functionExists(MiniMC::Support::getFunctionId(ptr));
            MiniMC::Support::Localiser function_not_exists("Call references unexisting function: '%1%'");
            if (!is_func) {
              mess.error(function_not_exists.format(MiniMC::Support::getFunctionId(ptr)));
              return false;
            }

            auto func = prgm->getFunction(MiniMC::Support::getFunctionId(ptr));
            if (func->getParameters().size() != h.nbParams()) {
              mess.error("Inconsistent number of parameters between call and function prototype");
              return false;
            }

            auto nbParams = h.nbParams();
            auto it = func->getParameters().begin();

            for (size_t i = 0; i < nbParams; i++, ++it) {
              auto form_type = (*it)->getType();
              auto act_type = h.getParam(i)->getType();
              if (form_type != act_type) {
                mess.error("Formal and actual parameters do not match typewise");
                return false;
              }
            }

            if (h.getResult()) {
              auto resType = h.getResult()->getType();
              if (resType != func->getReturnType()) {
                mess.error("Result and return type of functions must match.");
                return false;
              }
            }
          }

          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::Assign> {
        static const MiniMC::Model::InstructionCode OpCode = MiniMC::Model::InstructionCode::Assign;
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          MiniMC::Support::Localiser must_be_same_type("Result and assignee must be same type for '%1%' ");
          InstHelper<OpCode> h(inst);
          auto valT = h.getValue()->getType();
          auto resT = h.getValue()->getType();
          if (valT != resT) {
            mess.error(must_be_same_type.format(OpCode));
            return false;
          }
          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::Ret> {
        static const MiniMC::Model::InstructionCode OpCode = MiniMC::Model::InstructionCode::Ret;
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr& tt, MiniMC::Model::Program_ptr& prgm) {
          InstHelper<MiniMC::Model::InstructionCode::Ret> h(inst);

          if (tt != h.getValue()->getType()) {
            MiniMC::Support::Localiser must_be_same_type("Return value of '%1% must be the same as the function");

            mess.error(must_be_same_type.format(MiniMC::Model::InstructionCode::Ret));
            return false;
          }
          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::RetVoid> {
        static const MiniMC::Model::InstructionCode OpCode = MiniMC::Model::InstructionCode::Call;
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr& tt, MiniMC::Model::Program_ptr& prgm) {
          if (tt->getTypeID() != MiniMC::Model::TypeID::Void) {
            MiniMC::Support::Localiser must_be_same_type("Return type of function with '%1%' must be void  ");
            mess.error(must_be_same_type.format(MiniMC::Model::InstructionCode::RetVoid));
            return false;
          }
          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::NonDet> {
        static const MiniMC::Model::InstructionCode OpCode = MiniMC::Model::InstructionCode::NonDet;
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          InstHelper<OpCode> h(inst);
          auto type = h.getResult()->getType();
          MiniMC::Support::Localiser must_be_integer("'%1% must return Integers");
          if (type->getTypeID() != MiniMC::Model::TypeID::Integer) {
            mess.error(must_be_integer.format(OpCode));
            return false;
          }

          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::Assert> {
        static const MiniMC::Model::InstructionCode OpCode = MiniMC::Model::InstructionCode::Assert;
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          InstHelper<OpCode> h(inst);
          MiniMC::Support::Localiser must_be_bool("'%1%' must take boolean or integer as input. ");

          auto type = h.getAssert()->getType();
          if (type->getTypeID() != MiniMC::Model::TypeID::Bool &&
              type->getTypeID() != MiniMC::Model::TypeID::Integer) {
            mess.error(must_be_bool.format(OpCode));
            return false;
          }
          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::Assume> {
        static const MiniMC::Model::InstructionCode OpCode = MiniMC::Model::InstructionCode::Assume;
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          InstHelper<OpCode> h(inst);
          MiniMC::Support::Localiser must_be_bool("'%1%' must take boolean or integer as input. ");

          auto type = h.getAssert()->getType();
          if (type->getTypeID() != MiniMC::Model::TypeID::Bool &&
              type->getTypeID() != MiniMC::Model::TypeID::Integer) {
            mess.error(must_be_bool.format(OpCode));
            return false;
          }
          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::NegAssume> {
        static const MiniMC::Model::InstructionCode OpCode = MiniMC::Model::InstructionCode::NegAssume;
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          InstHelper<OpCode> h(inst);
          MiniMC::Support::Localiser must_be_bool("'%1%' must take boolean or integer as input. ");

          auto type = h.getAssert()->getType();
          if (type->getTypeID() != MiniMC::Model::TypeID::Bool &&
              type->getTypeID() != MiniMC::Model::TypeID::Integer) {
            mess.error(must_be_bool.format(OpCode));
            return false;
          }
          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::StackRestore> {
        static const MiniMC::Model::InstructionCode OpCode = MiniMC::Model::InstructionCode::StackRestore;
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          InstHelper<OpCode> h(inst);
          MiniMC::Support::Localiser must_be_pointer("'%1%' must take pointer as inputs. ");
          auto type = h.getValue()->getType();
          if (type->getTypeID() != MiniMC::Model::TypeID::Pointer) {
            mess.error(must_be_pointer.format(OpCode));
            return false;
          }
          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::MemCpy> {
        static const MiniMC::Model::InstructionCode OpCode = MiniMC::Model::InstructionCode::MemCpy;
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          InstHelper<OpCode> h(inst);
          MiniMC::Support::Localiser must_be_pointer_source("'%1%' must take pointer as source inputs. ");
          MiniMC::Support::Localiser must_be_pointer_target("'%1%' must take pointer as target inputs. ");
          MiniMC::Support::Localiser must_be_integer("'%1%' must take integer as size inputs. ");

          if (h.getSource()->getType()->getTypeID() != MiniMC::Model::TypeID::Pointer) {
            mess.error(must_be_pointer_source.format(OpCode));
            return false;
          }

          if (h.getTarget()->getType()->getTypeID() != MiniMC::Model::TypeID::Pointer) {
            mess.error(must_be_pointer_target.format(OpCode));
            return false;
          }

          if (h.getSize()->getType()->getTypeID() != MiniMC::Model::TypeID::Integer) {
            mess.error(must_be_integer.format(OpCode));
            return false;
          }

          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::StackSave> {
        static const MiniMC::Model::InstructionCode OpCode = MiniMC::Model::InstructionCode::StackSave;
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          InstHelper<OpCode> h(inst);
          MiniMC::Support::Localiser must_be_pointer("'%1%' must take pointer as result. ");
          auto type = h.getResult()->getType();
          if (type->getTypeID() != MiniMC::Model::TypeID::Pointer) {
            mess.error(must_be_pointer.format(OpCode));
            return false;
          }
          return true;
        }
      };

      template <>
      struct TypeCheck<MiniMC::Model::InstructionCode::Uniform> {
        static const MiniMC::Model::InstructionCode OpCode = MiniMC::Model::InstructionCode::Uniform;
        static bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program_ptr& prgm) {
          InstHelper<OpCode> h(inst);
          MiniMC::Support::Localiser must_be_same_type("Result and assignee must be same type for '%1%' ");
          MiniMC::Support::Localiser must_be_integer("Result must be integer for '%1%' ");

          if (!MiniMC::Model::hasSameTypeID({h.getResult()->getType(),
                                             h.getMax()->getType(),
                                             h.getMin()->getType()})) {
            mess.error(must_be_same_type.format(OpCode));
            return false;
          }

          if (h.getResult()->getType()->getTypeID() != MiniMC::Model::TypeID::Integer) {
            mess.error(must_be_integer.format(OpCode));
            return false;
          }

          return true;
        }
      };

      bool TypeChecker::run(MiniMC::Model::Program& prgm) {
        messager.message("Initiating Typechecking");
        Program_ptr prgm_ptr = prgm.shared_from_this();
        bool res = true;
        for (auto& F : prgm.getFunctions()) {
          for (auto& E : F->getCFG()->getEdges()) {
            if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
              for (auto& I : E->getAttribute<MiniMC::Model::AttributeType::Instructions>()) {

                switch (I.getOpcode()) {
#define X(OP)                                                                                                 \
  case MiniMC::Model::InstructionCode::OP:                                                                    \
    if (!TypeCheck<MiniMC::Model::InstructionCode::OP>::doCheck(I, messager, F->getReturnType(), prgm_ptr)) { \
      res = false;                                                                                            \
    }                                                                                                         \
    break;
                  TACOPS
                  COMPARISONS
                  CASTOPS
                  MEMORY
                  INTERNAL
		  POINTEROPS
                  AGGREGATEOPS
                  PREDICATES
                }
              }
            }
          }
        }
        if (!res) {
          messager.error("Type check not passing");
        }
        messager.message("Typechecking finished");
        return res;
      }
    } // namespace Checkers
  }   // namespace Model
} // namespace MiniMC
