#include "model/checkers/typechecker.hpp"

#include "model/cfg.hpp"
#include "support/feedback.hpp"
#include "support/localisation.hpp"

namespace MiniMC {
  namespace Model {
    namespace Checkers {
      template <MiniMC::Model::InstructionCode i>
      bool doCheck(MiniMC::Model::Instruction& inst, const MiniMC::Model::Type_ptr& tt, MiniMC::Model::Program& prgm) {
	MiniMC::Support::Messager mess;
        auto& content = inst.getOps<i>();
        if constexpr (InstructionData<i>::isTAC ||  i  == MiniMC::Model::InstructionCode::PtrEq) {
          MiniMC::Support::Localiser loc("All operands to '%1%' must have same type as the result.");
          auto resType = content.res->getType();
          auto lType = content.op1->getType();
          auto rType = content.op2->getType();
          if (resType != lType ||
              lType != rType ||
              rType != resType) {
            mess.message<MiniMC::Support::Severity::Error>(loc.format(i));
            return false;
          }
          return true;
        } else if constexpr (InstructionData<i>::isPredicate) {
          MiniMC::Support::Localiser loc("All operands to '%1%' must be same type.");
          auto lType = content.op1->getType();
          auto rType = content.op2->getType();
          if (lType != rType) {
            mess.message<MiniMC::Support::Severity::Error>(loc.format(i));
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
              mess.message<MiniMC::Support::Severity::Error>(loc.format(i));
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
            mess.message<MiniMC::Support::Severity::Error>(loc.format(i));
            return false;
          } else if (resType->getTypeID() != MiniMC::Model::TypeID::Bool) {
            mess.message<MiniMC::Support::Severity::Error>(res_must_be_bool.format(i));
            return false;
          }

          return true;
        }

        else if constexpr (i == InstructionCode::Trunc) {
          MiniMC::Support::Localiser trunc_must_be_integer("'%1%' can only be applied to integer types. ");
          MiniMC::Support::Localiser trunc_must_be_larger("From type must be larger that to type for '%1%'");

          auto ftype = content.op1->getType();
          auto ttype = content.res->getType();
          if (!ftype->isInteger () ||
              !ttype->isInteger () ) {
            mess.message<MiniMC::Support::Severity::Error>(trunc_must_be_integer.format(MiniMC::Model::InstructionCode::Trunc));
            return false;
          } else if (ftype->getSize() <= ttype->getSize()) {
            mess.message<MiniMC::Support::Severity::Error>(trunc_must_be_larger.format(MiniMC::Model::InstructionCode::Trunc));
            return false;
          }

          return true;
        }

        else if constexpr (i == InstructionCode::IntToBool) {
          MiniMC::Support::Localiser must_be_integer("'%1%' can only be applied to integer types. ");
          MiniMC::Support::Localiser must_be_smaller("From type must be smaller that to type for '%1%'");

          auto ftype = content.op1->getType();
          auto ttype = content.res->getType();

          if (!ftype->isInteger () ||
              ttype->getTypeID() != MiniMC::Model::TypeID::Bool) {
            mess.message<MiniMC::Support::Severity::Error>(must_be_integer.format(MiniMC::Model::InstructionCode::IntToBool));
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

          if (!ftype->isInteger () ||
              !ttype->isInteger () ) {
            mess.message<MiniMC::Support::Severity::Error>(must_be_integer.format(i));
            return false;
          } else if (ftype->getSize() > ttype->getSize()) {
            mess.message<MiniMC::Support::Severity::Error>(must_be_smaller.format(i));
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
              !ttype->isInteger () ) {
            mess.message<MiniMC::Support::Severity::Error>(must_be_integer.format(i));
            return false;
          } else if (ftype->getSize() > ttype->getSize()) {
            mess.message<MiniMC::Support::Severity::Error>(must_be_smaller.format(i));
            return false;
          }

          return true;
        }

        else if constexpr (i == InstructionCode::IntToPtr) {
          MiniMC::Support::Localiser must_be_integer("'%1%' can only be applied to integer types. ");
          MiniMC::Support::Localiser must_be_pointer("Return type has to be pointer for '%1%'");

          auto ftype = content.op1->getType();
          auto ttype = content.res->getType();

          if (!ftype->isInteger ()) {
            mess.message<MiniMC::Support::Severity::Error>(must_be_integer.format(MiniMC::Model::InstructionCode::IntToPtr));
            return false;
          }

          else if (ttype->getTypeID() != MiniMC::Model::TypeID::Pointer) {
            mess.message<MiniMC::Support::Severity::Error>(must_be_pointer.format(MiniMC::Model::InstructionCode::IntToPtr));
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
	  
          if (result->getTypeID() != MiniMC::Model::TypeID::Pointer &&
	      result->getTypeID() != MiniMC::Model::TypeID::Pointer32
	      ) {
            mess.message<MiniMC::Support::Severity::Error>(must_be_pointer.format(MiniMC::Model::InstructionCode::PtrAdd));
            return false;
          } else if (!skip->isInteger () ) {
            mess.message<MiniMC::Support::Severity::Error>(must_be_integer.format(MiniMC::Model::InstructionCode::PtrAdd, "SkipSize"));
            return false;
          } else if (!value->isInteger ()) {
            mess.message<MiniMC::Support::Severity::Error>(must_be_integer.format(MiniMC::Model::InstructionCode::PtrAdd, "Value"));
            return false;
          } else if (value != skip) {
            mess.message<MiniMC::Support::Severity::Error>(must_be_same_type.format(MiniMC::Model::InstructionCode::PtrAdd, "Value"));
            return false;
          }

          else if (ptr->getTypeID() != MiniMC::Model::TypeID::Pointer &&
		   ptr->getTypeID() != MiniMC::Model::TypeID::Pointer32
		   ) {
            mess.message<MiniMC::Support::Severity::Error>(base_must_be_pointer.format(MiniMC::Model::InstructionCode::PtrAdd));
            return false;
          }

	  if (ptr->getTypeID () != result->getTypeID ()) {
	    mess.message<MiniMC::Support::Severity::Error>("Must be ssame pointer type");
            return false;
          
	  }
	  
          return true;
        }

        else if constexpr (i == InstructionCode::PtrToInt) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only be applied to pointer types. ");
          MiniMC::Support::Localiser must_be_integer("Return type has to be integer for '%1%'");
	  
          auto ftype = content.op1->getType();
          auto ttype = content.res->getType();
          if (ftype->getTypeID() != MiniMC::Model::TypeID::Pointer &&
 	      ftype->getTypeID() != MiniMC::Model::TypeID::Pointer32
	      ) {
            mess.message<MiniMC::Support::Severity::Error>(must_be_pointer.format(MiniMC::Model::InstructionCode::PtrToInt));
            return false;
          }

          else if (!ttype->isInteger ()) {
            mess.message<MiniMC::Support::Severity::Error>(must_be_pointer.format(MiniMC::Model::InstructionCode::PtrToInt));
            return false;
          }

          return true;
        }
        
        else if constexpr (i == InstructionCode::Store) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only store to pointer types. ");

          auto addr = content.addr->getType();
          if (addr->getTypeID() != MiniMC::Model::TypeID::Pointer &&
	      addr->getTypeID() != MiniMC::Model::TypeID::Pointer32
	      ) {
            mess.message<MiniMC::Support::Severity::Error>(must_be_pointer.format(MiniMC::Model::InstructionCode::Store));
            return false;
          }

          return true;
        }
	
	
        else if constexpr (i == InstructionCode::Load) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only load from pointer types. ");
          MiniMC::Support::Localiser must_be_integer("'%1%' can only load integers. ");

          auto addr = content.addr->getType();
          if (addr->getTypeID() != MiniMC::Model::TypeID::Pointer &&
	      addr->getTypeID() != MiniMC::Model::TypeID::Pointer32) {
            mess.message<MiniMC::Support::Severity::Error>(must_be_pointer.format(MiniMC::Model::InstructionCode::Load));
            return false;
          }

          if (!content.res->getType()->isInteger () ) {
            mess.message<MiniMC::Support::Severity::Error>(must_be_integer.format(MiniMC::Model::InstructionCode::Load));
            return false;
          }

          return true;
        }

        else if constexpr (i == InstructionCode::Skip) {
          return true;
        }

	
        else if constexpr (i == InstructionCode::Skip) {
          return true;
        }
	
        else if constexpr (i == InstructionCode::Call) {
          auto func = content.function;
          if (!func->isConstant()) {
            MiniMC::Support::Localiser must_be_constant("'%1%' can only use constant function pointers. ");
            mess.message<MiniMC::Support::Severity::Error>(must_be_constant.format(i));
            return false;
          }

          else {
            auto constant = std::static_pointer_cast<MiniMC::Model::TConstant<pointer_t>>(func);
            auto ptr = (*constant).template getValue (); //MiniMC::Support::CastToPtr (constant->getValue());
            bool is_func = prgm.functionExists(MiniMC::getFunctionId(ptr));
            MiniMC::Support::Localiser function_not_exists("Call references unexisting function: '%1%'");
            if (!is_func) {
              mess.message<MiniMC::Support::Severity::Error>(function_not_exists.format(MiniMC::getFunctionId(ptr)));
              return false;
            }

            auto func = prgm.getFunction(MiniMC::getFunctionId(ptr));
            if (func->getParameters().size() != content.params.size()) {
              mess.message<MiniMC::Support::Severity::Error>("Inconsistent number of parameters between call and function prototype");
              return false;
            }

            auto nbParams = content.params.size();
            auto it = func->getParameters().begin();

            for (size_t j = 0; j < nbParams; j++, ++it) {
              auto form_type = (*it)->getType();
              auto act_type = content.params.at(j)->getType();
              if (form_type != act_type) {
                mess.message<MiniMC::Support::Severity::Error>("Formal and actual parameters do not match typewise");
                return false;
              }
            }

            if (content.res) {
              auto resType = content.res->getType();
              if (resType != func->getReturnType()) {
                mess.message<MiniMC::Support::Severity::Error>("Result and return type of functions must match.");
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
            mess.message<MiniMC::Support::Severity::Error>(must_be_same_type.format(i));
            return false;
          }
          return true;
        }

        else if constexpr (i == InstructionCode::Ret) {
          if (tt != content.value->getType()) {
            MiniMC::Support::Localiser must_be_same_type("Return value of '%1% must be the same as the function");

            mess.message<MiniMC::Support::Severity::Error>(must_be_same_type.format(MiniMC::Model::InstructionCode::Ret));
            return false;
          }
          return true;
        }

        else if constexpr (i == InstructionCode::RetVoid) {
          if (tt->getTypeID() != MiniMC::Model::TypeID::Void) {
            MiniMC::Support::Localiser must_be_same_type("Return type of function with '%1%' must be void  ");
            mess.message<MiniMC::Support::Severity::Error>(must_be_same_type.format(MiniMC::Model::InstructionCode::RetVoid));
            return false;
          }
          return true;
        }

        else if constexpr (i == InstructionCode::NonDet) {
          auto type = inst.getOps<i>().res->getType();
          MiniMC::Support::Localiser must_be_integer("'%1% must return Integers");
          if (!type->isInteger ()) {
            mess.message<MiniMC::Support::Severity::Error>(must_be_integer.format(i));
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
              !type->isInteger () ) {
            mess.message<MiniMC::Support::Severity::Error>(must_be_bool.format(i));
            return false;
          }
          return true;
        }

	else if constexpr (i == InstructionCode::ExtractValue ||
                           i == InstructionCode::InsertValue
			   ) {
          MiniMC::Support::Localiser warning("TypeCheck not fully implemented for '%1%'");
	  MiniMC::Support::Localiser must_be_aggregate("%2% must be aggregate '%1%'");
	  MiniMC::Support::Localiser must_be_integer("offset must be integer '%1%'");
	  
	  if (!content.offset->getType ()->isInteger ()) {
            mess.message<MiniMC::Support::Severity::Error>(must_be_integer.format(i));
            return false;
          }

	  if (!content.aggregate->getType ()->isAggregate ()) {
            mess.message<MiniMC::Support::Severity::Error>(must_be_aggregate.format(i,"aggregate"));
            return false;
          }

	  if constexpr (i == InstructionCode::InsertValue) {
	    if (!content.res->getType ()->isAggregate ()) {
	      mess.message<MiniMC::Support::Severity::Error>(must_be_aggregate.format(i,"res"));
	      return false;
	    }
	  }
	  
	  return true;
        }
        
        else if constexpr (i == InstructionCode::BitCast) {
          MiniMC::Support::Localiser warning("TypeCheck not fully implemented for '%1%'");
	  mess.message<MiniMC::Support::Severity::Warning> (warning.format(i));
          return true;
        }

        else if constexpr (i == InstructionCode::Uniform) {
          MiniMC::Support::Localiser must_be_same_type("Result and assignee must be same type for '%1%' ");
          MiniMC::Support::Localiser must_be_integer("Result must be integer for '%1%' ");

          if (!MiniMC::Model::hasSameTypeID({content.res->getType(),
                                             content.max->getType(),
                                             content.min->getType()})) {
            mess.message<MiniMC::Support::Severity::Error>(must_be_same_type.format(i));
            return false;
          }

          if (!content.res->getType()->isInteger ()) {
            mess.message<MiniMC::Support::Severity::Error>(must_be_integer.format(i));
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
        bool res = true;
        for (auto& F : prgm.getFunctions()) {
          for (auto& E : F->getCFA().getEdges()) {
	    auto& instrkeeper = E->getInstructions ();
            if (instrkeeper) {
              for (auto& I : instrkeeper) {

                switch (I.getOpcode()) {
#define X(OP)                                                                                      \
  case MiniMC::Model::InstructionCode::OP:                                                         \
    if (!doCheck<MiniMC::Model::InstructionCode::OP>(I,  F->getReturnType(), prgm)) { \
      res = false;                                                                                 \
    }                                                                                              \
    break;
                  OPERATIONS
                }
              }
            }
          }
        }
        return res;
      }
    } // namespace Checkers
  }   // namespace Model
} // namespace MiniMC
