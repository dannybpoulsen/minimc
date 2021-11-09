#include "model/checkers/typechecker.hpp"

#include "model/cfg.hpp"
#include "support/feedback.hpp"
#include "support/localisation.hpp"
#include "support/pointer.hpp"

namespace MiniMC {
  namespace Model {
    namespace Checkers {
      template <MiniMC::Model::InstructionCode i>
      bool doCheck(MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess, const MiniMC::Model::Type_ptr& tt, MiniMC::Model::Program_ptr& prgm) {
        auto& content = inst.getOps<i>();
        if constexpr (InstructionData<i>::isTAC) {
          MiniMC::Support::Localiser loc("All operands to '%1%' must have same type as the result.");
          auto resType = content.res->getType();
          auto lType = content.op1->getType();
          auto rType = content.op2->getType();
          if (resType != lType ||
              lType != rType ||
              rType != resType) {
            mess.error(loc.format(i));
            return false;
          }
          return true;
        } else if constexpr (InstructionData<i>::isPredicate) {
          MiniMC::Support::Localiser loc("All operands to '%1%' must be same type.");
          auto lType = content.op1->getType();
          auto rType = content.op2->getType();
          if (lType != rType) {
            mess.error(loc.format(i));
            return false;
          }
          return true;
        }

        else if constexpr (InstructionData<i>::isUnary) {
          if constexpr (i == MiniMC::Model::InstructionCode::Not) {
            MiniMC::Support::Localiser loc("All operands to '%1%' must have same type as the result.");
            auto resType = content.res->getType();
            auto lType = content.op1->getType();
            if (resType != lType) {
              mess.error(loc.format(i));
              return false;
            }
            return true;
          }
        }

        else if constexpr (InstructionData<i>::isComparison) {
          MiniMC::Support::Localiser loc("All operands to '%1%' must have same type..");
          MiniMC::Support::Localiser res_must_be_bool("The result of '%1% must be boolean.");

          auto resType = content.res->getType();
          auto lType = content.op1->getType();
          auto rType = content.op2->getType();
          if (lType != rType) {
            mess.error(loc.format(i));
            return false;
          } else if (resType->getTypeID() != MiniMC::Model::TypeID::Bool) {
            mess.error(res_must_be_bool.format(i));
            return false;
          }

          return true;
        }

        else if constexpr (i == InstructionCode::Trunc) {
          MiniMC::Support::Localiser trunc_must_be_integer("'%1%' can only be applied to integer types. ");
          MiniMC::Support::Localiser trunc_must_be_larger("From type must be larger that to type for '%1%'");

          auto ftype = content.op1->getType();
          auto ttype = content.res->getType();
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

        else if constexpr (i == InstructionCode::IntToBool) {
          MiniMC::Support::Localiser must_be_integer("'%1%' can only be applied to integer types. ");
          MiniMC::Support::Localiser must_be_smaller("From type must be smaller that to type for '%1%'");

          auto ftype = content.op1->getType();
          auto ttype = content.res->getType();

          if (ftype->getTypeID() != MiniMC::Model::TypeID::Integer ||
              ttype->getTypeID() != MiniMC::Model::TypeID::Bool) {
            mess.error(must_be_integer.format(MiniMC::Model::InstructionCode::IntToBool));
            return false;
          }
          return true;
        }

        else if constexpr (i == InstructionCode::SExt ||
                           i == InstructionCode::ZExt) {
          MiniMC::Support::Localiser must_be_integer("'%1%' can only be applied to integer types. ");
          MiniMC::Support::Localiser must_be_smaller("From type must be smaller that to type for '%1%'");

          auto ftype = content.op1->getType();
          auto ttype = content.res->getType();

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

        else if constexpr (i == InstructionCode::BoolSExt ||
                           i == InstructionCode::BoolZExt) {

          MiniMC::Support::Localiser must_be_integer("'%1%' can only be applied from boolean types to  integer types. ");
          MiniMC::Support::Localiser must_be_smaller("From type must be smaller that to type for '%1%'");

          auto ftype = content.op1->getType();
          auto ttype = content.res->getType();

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

        else if constexpr (i == InstructionCode::IntToPtr) {
          MiniMC::Support::Localiser must_be_integer("'%1%' can only be applied to integer types. ");
          MiniMC::Support::Localiser must_be_pointer("Return type has to be pointer for '%1%'");

          auto ftype = content.op1->getType();
          auto ttype = content.res->getType();

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

        else if constexpr (i == InstructionCode::PtrAdd) {
          MiniMC::Support::Localiser must_be_integer("'%2%' has to be an integer for '%1%'. ");
          MiniMC::Support::Localiser must_be_same_type("'value and skipeSize must be same type '%1%'. ");

          MiniMC::Support::Localiser must_be_pointer("Return type has to be pointer for '%1%'");
          MiniMC::Support::Localiser base_must_be_pointer("Base  has to be pointer for '%1%'");

          auto ptr = content.ptr->getType();
          auto skip = content.skipsize->getType();
          auto value = content.nbSkips->getType();
          auto result = content.res->getType();

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

        else if constexpr (i == InstructionCode::PtrToInt) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only be applied to pointer types. ");
          MiniMC::Support::Localiser must_be_integer("Return type has to be integer for '%1%'");

          auto ftype = content.op1->getType();
          auto ttype = content.res->getType();
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

        else if constexpr (i == InstructionCode::Alloca) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only return pointer types. ");
          MiniMC::Support::Localiser must_be_integer("Size parameter to '%1%' must be an Integer and 8 bytes long. ");

          auto restype = content.res->getType();
          auto sizetype = content.op1->getType();

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

        else if constexpr (i == InstructionCode::ExtendObj) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only return pointer types. ");
          MiniMC::Support::Localiser must_be_integer("Size parameter to '%1%' must be an Integer and 8 bytes long. ");

          auto restype = content.res->getType();
          auto pointertype = content.object->getType();
          auto sizetype = content.size->getType();

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

        else if constexpr (i == InstructionCode::Malloc) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only accept pointer types. ");
          MiniMC::Support::Localiser must_be_integer("Size parameter to '%1%' must be an Integer and 8 bytes long. ");

          auto ptrtype = content.object->getType();
          auto sizetype = content.size->getType();
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

        else if constexpr (i == InstructionCode::Free) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only accept pointer types. ");
          auto ptrtype = content.object->getType();
  
          if (ptrtype->getTypeID() != MiniMC::Model::TypeID::Pointer) {
            mess.error(must_be_pointer.format(MiniMC::Model::InstructionCode::Malloc));
            return false;
          }

          return true;
        }

        else if constexpr (i == InstructionCode::FindSpace) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only return pointer types. ");
          MiniMC::Support::Localiser must_be_integer("Size parameter to '%1%' must be an Integer. ");

          auto restype = content.res->getType();
          auto sizetype = content.op1->getType();

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

        else if constexpr (i == InstructionCode::Store) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only store to pointer types. ");

          auto addr = content.addr->getType();
          if (addr->getTypeID() != MiniMC::Model::TypeID::Pointer) {
            mess.error(must_be_pointer.format(MiniMC::Model::InstructionCode::Store));
            return false;
          }

          return true;
        }

        else if constexpr (i == InstructionCode::Load) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only load from pointer types. ");
          MiniMC::Support::Localiser must_be_integer("'%1%' can only load integers. ");

          auto addr = content.addr->getType();
          if (addr->getTypeID() != MiniMC::Model::TypeID::Pointer) {
            mess.error(must_be_pointer.format(MiniMC::Model::InstructionCode::Load));
            return false;
          }

          if (content.res->getType()->getTypeID() != MiniMC::Model::TypeID::Integer) {
            mess.error(must_be_integer.format(MiniMC::Model::InstructionCode::Load));
            return false;
          }

          return true;
        }

        else if constexpr (i == InstructionCode::Skip) {
          return true;
        }

        else if constexpr (i == InstructionCode::Call) {
          //auto prgm = inst.getFunction()->getPrgm ();;
          auto func = content.function;
          if (!func->isConstant()) {
            MiniMC::Support::Localiser must_be_constant("'%1%' can only use constant function pointers. ");
            mess.error(must_be_constant.format(i));
            return false;
          }

          else {
            auto constant = std::static_pointer_cast<MiniMC::Model::BinaryBlobConstant>(func);
            auto ptr = (*constant).template getValue<MiniMC::pointer_t>(); //MiniMC::Support::CastToPtr (constant->getValue());
            bool is_func = prgm->functionExists(MiniMC::Support::getFunctionId(ptr));
            MiniMC::Support::Localiser function_not_exists("Call references unexisting function: '%1%'");
            if (!is_func) {
              mess.error(function_not_exists.format(MiniMC::Support::getFunctionId(ptr)));
              return false;
            }

            auto func = prgm->getFunction(MiniMC::Support::getFunctionId(ptr));
            if (func->getParameters().size() != content.params.size()) {
              mess.error("Inconsistent number of parameters between call and function prototype");
              return false;
            }

            auto nbParams = content.params.size();
            auto it = func->getParameters().begin();

            for (size_t j = 0; j < nbParams; j++, ++it) {
              auto form_type = (*it)->getType();
              auto act_type = content.params.at(j)->getType();
              if (form_type != act_type) {
                mess.error("Formal and actual parameters do not match typewise");
                return false;
              }
            }

            if (content.res) {
              auto resType = content.res->getType();
              if (resType != func->getReturnType()) {
                mess.error("Result and return type of functions must match.");
                return false;
              }
            }
          }

          return true;
        }

        else if constexpr (i == InstructionCode::Assign) {
          MiniMC::Support::Localiser must_be_same_type("Result and assignee must be same type for '%1%' ");
          auto valT = content.op1->getType();
          auto resT = content.res->getType();
          if (valT != resT) {
            mess.error(must_be_same_type.format(i));
            return false;
          }
          return true;
        }

        else if constexpr (i == InstructionCode::Ret) {
          if (tt != content.value->getType()) {
            MiniMC::Support::Localiser must_be_same_type("Return value of '%1% must be the same as the function");

            mess.error(must_be_same_type.format(MiniMC::Model::InstructionCode::Ret));
            return false;
          }
          return true;
        }

        else if constexpr (i == InstructionCode::RetVoid) {
          if (tt->getTypeID() != MiniMC::Model::TypeID::Void) {
            MiniMC::Support::Localiser must_be_same_type("Return type of function with '%1%' must be void  ");
            mess.error(must_be_same_type.format(MiniMC::Model::InstructionCode::RetVoid));
            return false;
          }
          return true;
        }

        else if constexpr (i == InstructionCode::NonDet) {
          auto type = inst.getOps<i>().res->getType();
          MiniMC::Support::Localiser must_be_integer("'%1% must return Integers");
          if (type->getTypeID() != MiniMC::Model::TypeID::Integer) {
            mess.error(must_be_integer.format(i));
            return false;
          }

          return true;
        }

        else if constexpr (i == InstructionCode::Assert ||
                           i == InstructionCode::Assume ||
                           i == InstructionCode::NegAssume) {
          MiniMC::Support::Localiser must_be_bool("'%1%' must take boolean or integer as input. ");

          auto type = content.expr->getType();
          if (type->getTypeID() != MiniMC::Model::TypeID::Bool &&
              type->getTypeID() != MiniMC::Model::TypeID::Integer) {
            mess.error(must_be_bool.format(i));
            return false;
          }
          return true;
        }

        else if constexpr (i == InstructionCode::StackRestore) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' must take pointer as inputs. ");
          auto type = content.stackobject->getType();
          if (type->getTypeID() != MiniMC::Model::TypeID::Pointer) {
            mess.error(must_be_pointer.format(i));
            return false;
          }
          return true;
        }

        else if constexpr (i == InstructionCode::MemCpy) {
          MiniMC::Support::Localiser must_be_pointer_source("'%1%' must take pointer as source inputs. ");
          MiniMC::Support::Localiser must_be_pointer_target("'%1%' must take pointer as target inputs. ");
          MiniMC::Support::Localiser must_be_integer("'%1%' must take integer as size inputs. ");

          if (content.src->getType()->getTypeID() != MiniMC::Model::TypeID::Pointer) {
            mess.error(must_be_pointer_source.format(i));
            return false;
          }

          if (content.dst->getType()->getTypeID() != MiniMC::Model::TypeID::Pointer) {
            mess.error(must_be_pointer_target.format(i));
            return false;
          }

          if (content.size->getType()->getTypeID() != MiniMC::Model::TypeID::Integer) {
            mess.error(must_be_integer.format(i));
            return false;
          }

          return true;
        }

        else if constexpr (i == InstructionCode::StackSave) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' must take pointer as result. ");
          auto type = content.res->getType();
          if (type->getTypeID() != MiniMC::Model::TypeID::Pointer) {
            mess.error(must_be_pointer.format(i));
            return false;
          }
          return true;
        }
      };

        else if constexpr (i == InstructionCode::BitCast ||
                           i == InstructionCode::ExtractValue ||
                           i == InstructionCode::InsertValue) {
          MiniMC::Support::Localiser warning("TypeCheck not fully implemented for '%1%'");
          mess.warning(warning.format(i));
          return true;
        }

        else if constexpr (i == InstructionCode::Uniform) {
          MiniMC::Support::Localiser must_be_same_type("Result and assignee must be same type for '%1%' ");
          MiniMC::Support::Localiser must_be_integer("Result must be integer for '%1%' ");

          if (!MiniMC::Model::hasSameTypeID({content.res->getType(),
                                             content.max->getType(),
                                             content.min->getType()})) {
            mess.error(must_be_same_type.format(i));
            return false;
          }

          if (content.res->getType()->getTypeID() != MiniMC::Model::TypeID::Integer) {
            mess.error(must_be_integer.format(i));
            return false;
          }

          return true;
        }

        else {
          []<bool b = false>() { static_assert(b && "cannot type check instruction"); }
          ();
        }
      }

      bool TypeChecker::run(MiniMC::Model::Program& prgm) {
        messager.message("Initiating Typechecking");
        Program_ptr prgm_ptr = prgm.shared_from_this();
        bool res = true;
        for (auto& F : prgm.getFunctions()) {
          for (auto& E : F->getCFG()->getEdges()) {
            if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
              for (auto& I : E->getAttribute<MiniMC::Model::AttributeType::Instructions>()) {

                switch (I.getOpcode()) {
#define X(OP)                                                                                      \
  case MiniMC::Model::InstructionCode::OP:                                                         \
    if (!doCheck<MiniMC::Model::InstructionCode::OP>(I, messager, F->getReturnType(), prgm_ptr)) { \
      res = false;                                                                                 \
    }                                                                                              \
    break;
                  OPERATIONS
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
