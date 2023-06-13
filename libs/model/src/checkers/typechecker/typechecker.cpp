#include "model/checkers/typechecker.hpp"

#include "model/cfg.hpp"
#include "model/instructions.hpp"
#include "model/valuevisitor.hpp"
#include "model/variables.hpp"
#include "support/feedback.hpp"
#include "support/localisation.hpp"

namespace MiniMC {
  namespace Model {
    namespace Checkers {

      class TypeError : public MiniMC::Support::ErrorMessage {
      public:
	TypeError (MiniMC::Model::Instruction instr) : instr(std::move(instr)) {}

      protected:
	auto& getInstr () const {return instr;}
	private:
	MiniMC::Model::Instruction instr;
	
      };

      class MustBeSameType : public TypeError{
      public:
	MustBeSameType (MiniMC::Model::Instruction instr, MiniMC::Model::Value_ptr l1, MiniMC::Model::Value_ptr l2 ) : TypeError (std::move(instr)),l1(l1),l2(l2) {}
	virtual std::ostream& to_string (std::ostream& os) const {
	  return os << loc.format (getInstr (),*l1,*l2);
	}
	
      private:
	MiniMC::Support::Localiser loc{"For Instruction '%1%': '%2%' and '%3%' must have same type."};
	MiniMC::Model::Value_ptr l1;
	MiniMC::Model::Value_ptr l2;
      };

      class MustBeGivenTypeID : public TypeError{
      public:
	MustBeGivenTypeID (MiniMC::Model::Instruction instr, MiniMC::Model::Value_ptr l1, MiniMC::Model::TypeID type ) : TypeError (std::move(instr)),l1(l1),type(type) {}
	virtual std::ostream& to_string (std::ostream& os) const {
	  return os << loc.format (getInstr (),*l1,type);
	}
	
      private:
	MiniMC::Support::Localiser loc{"For Instruction '%1%': '%2%' must be of type '%3%'."};
	MiniMC::Model::Value_ptr l1;
	MiniMC::Model::TypeID type;
      };

      class MustBeGivenType : public TypeError{
      public:
	MustBeGivenType (MiniMC::Model::Instruction instr, MiniMC::Model::Value_ptr l1, MiniMC::Model::Type_ptr type ) : TypeError (std::move(instr)),l1(l1),type(type) {}
	virtual std::ostream& to_string (std::ostream& os) const {
	  return os << loc.format (getInstr (),*l1,*type);
	}
	
      private:
	MiniMC::Support::Localiser loc{"For Instruction '%1%': '%2%' must be of type '%3%'."};
	MiniMC::Model::Value_ptr l1;
	MiniMC::Model::Type_ptr type;
      };
      
      class MustBeInteger : public TypeError{
      public:
	MustBeInteger (MiniMC::Model::Instruction instr, MiniMC::Model::Value_ptr l1 ) : TypeError (std::move(instr)),l1(l1) {}
	virtual std::ostream& to_string (std::ostream& os) const {
	  return os << loc.format (getInstr (),*l1);
	}
	
      private:
	MiniMC::Support::Localiser loc{"For Instruction '%1%': '%2%' must be an integer type."};
	MiniMC::Model::Value_ptr l1;
      };

      class MustBeLarger : public TypeError{
      public:
	MustBeLarger (MiniMC::Model::Instruction instr, MiniMC::Model::Type_ptr t1,MiniMC::Model::Type_ptr t2 ) : TypeError (std::move(instr)),t1(t1),t2(t2) {}
	virtual std::ostream& to_string (std::ostream& os) const {
	  return os << loc.format (getInstr (),*t1,*t2);
	}
	
      private:
	MiniMC::Support::Localiser loc{"For Instruction '%1%': '%2%' must be of larger size thant %3%"};
	MiniMC::Model::Type_ptr t1;
	MiniMC::Model::Type_ptr t2;
	
      };

      class MustBeConstant : public TypeError {
      public:
	MustBeConstant (MiniMC::Model::Instruction instr, MiniMC::Model::Value_ptr t1 ) : TypeError (std::move(instr)),val(t1) {}
	virtual std::ostream& to_string (std::ostream& os) const {
	  return os << loc.format (getInstr (),*val);
	}
      private:
	MiniMC::Support::Localiser loc{"For Instruction '%1%': '%2%' must be constant"};
	
	Value_ptr val;
      };

   
      
      template <class Inst>
      bool doCheck(const Inst& tinst, const MiniMC::Model::Instruction& inst, const MiniMC::Model::Type_ptr& tt, MiniMC::Model::Program& prgm,MiniMC::Support::Messager& mess) {
	constexpr auto i = tinst.getOpcode ();
	if constexpr (InstructionData<i>::isTAC ||  i  == MiniMC::Model::InstructionCode::PtrEq) {
	  auto& content = tinst.getOps ();
	  auto resType = content.res->getType();
          auto lType = content.op1->getType();
          auto rType = content.op2->getType();
          if (resType != lType ||
              lType != rType ||
              rType != resType) {
            mess << MustBeSameType {inst,content.op1,content.op2};
            return false;
          }
          return true;
        } else if constexpr (InstructionData<i>::isPredicate) {
	  auto& content = tinst.getOps ();
	  auto lType = content.op1->getType();
          auto rType = content.op2->getType();
          if (lType != rType) {
            mess << MustBeSameType {inst,content.op1,content.op2};
            return false;
          }
          return true;
        }

        else if constexpr (InstructionData<i>::isUnary) {
	  auto& content = tinst.getOps ();
	  if constexpr (i == MiniMC::Model::InstructionCode::Not) {
            auto resType = content.res->getType();
            auto lType = content.op1->getType();
            if (resType != lType) {
              mess << MustBeSameType {inst,content.res,content.op1};
              return false;
            }
            return true;
          }
        }

        else if constexpr (InstructionData<i>::isComparison) {
	  auto& content = tinst.getOps ();
	  MiniMC::Support::Localiser res_must_be_bool("The result of '%1% must be boolean.");
	  MiniMC::Support::Localiser  must_be_integers("Comparisons must be integers");
	  
	  
          auto resType = content.res->getType();
          auto lType = content.op1->getType();
          auto rType = content.op2->getType();
          if (lType != rType) {
            mess << MustBeSameType {inst,content.op1,content.op2};
            return false;
          }
	  
	  else if (resType->getTypeID() != MiniMC::Model::TypeID::Bool) {
            mess << MustBeGivenTypeID (inst,content.res,MiniMC::Model::TypeID::Bool);
            return false;
          }

          return true;
        }

        else if constexpr (i == InstructionCode::Trunc) {
	  auto& content = tinst.getOps ();
          auto ftype = content.op1->getType();
          auto ttype = content.res->getType();

	  if (!ftype->isInteger ()) {
	    mess << MustBeInteger (inst,content.op1);
	    return false;
	  }
	  if (!ttype->isInteger ()) {
	    mess << MustBeInteger (inst,content.op1);
	    return false;
	  }
	  if (ftype->getSize() <= ttype->getSize()) {
	    mess << MustBeLarger {inst,ttype,ftype};
	    return false;
          }

          return true;
        }

        else if constexpr (i == InstructionCode::IntToBool) {
	  auto& content = tinst.getOps ();
	  auto ftype = content.op1->getType();
          auto ttype = content.res->getType();

	  if (!ftype->isInteger ()) {
	    mess << MustBeInteger {inst,content.op1};
	    return false;
	  }

	  if (ttype->getTypeID() != MiniMC::Model::TypeID::Bool) {
	    mess << MustBeGivenTypeID {inst,content.res,MiniMC::Model::TypeID::Bool};
	    return false;
	  }
	  
          return true;
        }

        else if constexpr (i == InstructionCode::SExt ||
                           i == InstructionCode::ZExt) {
	  auto& content = tinst.getOps ();
	  auto ftype = content.op1->getType();
          auto ttype = content.res->getType();

          if (!ftype->isInteger ()) {
	    mess << MustBeInteger (inst,content.op1);
	    return false;
	  }
	  if (!ttype->isInteger ()) {
	    mess << MustBeInteger (inst,content.op1);
	    return false;
	  }
	  if (ftype->getSize() >= ttype->getSize()) {
	    mess << MustBeLarger {inst,ttype,ftype};
	    return false;
          }
          return true;
        }

        else if constexpr (i == InstructionCode::BoolSExt ||
                           i == InstructionCode::BoolZExt) {
	  auto& content = tinst.getOps ();
          MiniMC::Support::Localiser must_be_integer("'%1%' can only be applied from boolean types to  integer types. ");
          MiniMC::Support::Localiser must_be_smaller("From type must be smaller that to type for '%1%'");

          auto ftype = content.op1->getType();
          auto ttype = content.res->getType();

	  if (ftype->getTypeID () != MiniMC::Model::TypeID::Bool) {
	    mess << MustBeGivenTypeID (inst,content.op1,MiniMC::Model::TypeID::Bool);
	    return false;
	  }
	  
          if (!ttype->isInteger () ) {
	    mess << MustBeInteger {inst,content.res};;
            return false;
          } 

          return true;
        }

        else if constexpr (i == InstructionCode::IntToPtr) {
	  auto& content = tinst.getOps ();
	  MiniMC::Support::Localiser must_be_integer("'%1%' can only be applied to integer types. ");
          MiniMC::Support::Localiser must_be_pointer("Return type has to be pointer for '%1%'");

          auto ftype = content.op1->getType();
          auto ttype = content.res->getType();

          if (!ftype->isInteger ()) {
	    mess << MustBeInteger {inst,content.op1};
            return false;
          }

          else if (ttype->getTypeID() != MiniMC::Model::TypeID::Pointer) {
	    mess << MustBeGivenTypeID (inst,content.res,MiniMC::Model::TypeID::Pointer);
	    return false;
          }

          return true;

        }
	
        else if constexpr (i == InstructionCode::PtrAdd ||i == InstructionCode::PtrSub ) {
          MiniMC::Support::Localiser must_be_integer("'%2%' has to be an integer for '%1%'. ");
          MiniMC::Support::Localiser must_be_same_type("'value and skipeSize must be same type '%1%'. ");

          MiniMC::Support::Localiser must_be_pointer("Return type has to be pointer for '%1%'");
          MiniMC::Support::Localiser base_must_be_pointer("Base  has to be pointer for '%1%'");
	  
	  auto& content = tinst.getOps ();
          auto ptr = content.ptr->getType();
          auto skip = content.skipsize->getType();
          auto value = content.nbSkips->getType();
          auto result = content.res->getType();
	  
          if (result->getTypeID() != MiniMC::Model::TypeID::Pointer &&
	      result->getTypeID() != MiniMC::Model::TypeID::Pointer32
	      ) {
            mess << MustBeGivenTypeID (inst,content.res,MiniMC::Model::TypeID::Pointer);
	    return false;
          }
	  if (!skip->isInteger () ) {
	    mess << MustBeInteger (inst,content.skipsize);
	    return false;
          }
	  if (!value->isInteger ()) {
	    mess << MustBeInteger (inst,content.nbSkips);
	    return false;
          }
	  if (value != skip) {
	    mess << MustBeSameType {inst,content.skipsize,content.nbSkips};
	    
            return false;
          }
	  
          if (ptr->getTypeID() != MiniMC::Model::TypeID::Pointer &&
	      ptr->getTypeID() != MiniMC::Model::TypeID::Pointer32
	      ) {
	    mess << MustBeGivenTypeID (inst,content.ptr,MiniMC::Model::TypeID::Pointer);
	    
	    
            return false;
          }

	  if (ptr->getTypeID () != result->getTypeID ()) {
	    mess << MustBeSameType {inst,content.ptr,content.res};
	    return false;
          
	  }
	  
          return true;
        }

        else if constexpr (i == InstructionCode::PtrToInt) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only be applied to pointer types. ");
          MiniMC::Support::Localiser must_be_integer("Return type has to be integer for '%1%'");

	  auto& content = tinst.getOps ();
          auto ftype = content.op1->getType();
          auto ttype = content.res->getType();
          if (ftype->getTypeID() != MiniMC::Model::TypeID::Pointer &&
 	      ftype->getTypeID() != MiniMC::Model::TypeID::Pointer32
	      ) {
	    mess << MustBeGivenTypeID (inst,content.op1,MiniMC::Model::TypeID::Pointer);
            return false;
          }

          else if (!ttype->isInteger ()) {
	    mess << MustBeInteger (inst,content.res);
            return false;
          }

          return true;
        }
        
        else if constexpr (i == InstructionCode::Store) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only store to pointer types. ");

	  auto& content = tinst.getOps ();
          auto addr = content.addr->getType();
          if (addr->getTypeID() != MiniMC::Model::TypeID::Pointer &&
	      addr->getTypeID() != MiniMC::Model::TypeID::Pointer32
	      ) {
	    mess << MustBeGivenTypeID (inst,content.addr,MiniMC::Model::TypeID::Pointer);
            return false;
          }

          return true;
        }
	
	
        else if constexpr (i == InstructionCode::Load) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only load from pointer types. ");
          MiniMC::Support::Localiser must_be_integer_or_pointer("'%1%' can only load integers or pointers ");

	  auto& content = tinst.getOps ();
          auto addr = content.addr->getType();
          if (addr->getTypeID() != MiniMC::Model::TypeID::Pointer &&
	      addr->getTypeID() != MiniMC::Model::TypeID::Pointer32) {
	    mess << MustBeGivenTypeID (inst,content.addr,MiniMC::Model::TypeID::Pointer);
            
	    
            return false;
          }

          if (!(content.res->getType()->isInteger () ||
		content.res->getType ()->getTypeID () == MiniMC::Model::TypeID::Pointer ||
		content.res->getType ()->getTypeID () == MiniMC::Model::TypeID::Pointer32
		)
	      ){
	    mess << MustBeInteger (inst,content.res);
            
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
	  MiniMC::Support::Localiser function_not_exists("Call references unexisting function: '%1%'");
	  MiniMC::Support::Localiser function_is_var_args("Call to var_args_functions '%1%'. Skipping parameter compatibility.");
	  auto& content = tinst.getOps ();
	  auto fun = content.function;
	  auto func = MiniMC::Model::visitValue(
						
						MiniMC::Model::Overload{
						  [&prgm,&function_not_exists,&mess](const MiniMC::Model::Pointer& val) -> Function_ptr {
						    auto ptr = val.getValue ();
						    if (prgm.functionExists (ptr.base))
						      return prgm.getFunction (ptr.base);
						    else {
						      mess << MiniMC::Support::TError<std::string> {function_not_exists.format(MiniMC::getFunctionId(ptr))};
						      return nullptr;
						    }
						  },
						    [&prgm,&mess,&function_not_exists](const MiniMC::Model::Pointer32& val) -> Function_ptr {
						      auto ptr = val.getValue ();
						      if (prgm.functionExists (ptr.base))
							return prgm.getFunction (ptr.base);
						      else {
							mess << MiniMC::Support::TError<std::string> {function_not_exists.format(ptr.base)};
							return nullptr;
						      }
						    },
						    [&prgm](const MiniMC::Model::SymbolicConstant& sc) -> Function_ptr {
						      return prgm.getFunction (sc.getValue ());
						    },
						    [&fun,&inst,&mess](const auto&) -> Function_ptr {
						      mess << MustBeConstant {inst,fun};
						      return nullptr;
						    }
						    },
						*fun
						);
	    
	  
	  if (!fun) {
	    return false;
	  }
	  
	  MiniMC::Support::Localiser inconsistent_parameters("Inconsistent number of parameters between call and function prototype '%1%'");
	  if (!func->isVarArgs ()) {
	    if (func->getParameters().size() != content.params.size() ) {
	      mess << MiniMC::Support::TError<std::string> {inconsistent_parameters.format (func->getSymbol ().getName ())};
	      return false;
	    }
	    
	    auto nbParams = content.params.size();
	    auto it = func->getParameters().begin();
	    
	    for (size_t j = 0; j < nbParams; j++, ++it) {
	      auto form_type = (*it)->getType();
	      auto act_type = content.params.at(j)->getType();
	      if (form_type != act_type) {
		mess << MustBeGivenType {inst,*it,act_type};

		return false;
	      }
	    }
	  }
	  else {
	    mess << MiniMC::Support::TWarning<std::string> {function_is_var_args.format (func->getSymbol ().getName ())};
	  }
	  if (content.res) {
	    auto resType = content.res->getType();
	    if (resType != func->getReturnType()) {
	      mess << MiniMC::Support::TError {inconsistent_parameters.format (func->getSymbol ().getName ())};
	      return false;
	    }
	  }
	
	return true;
	}

        else if constexpr (i == InstructionCode::Assign) {
	  auto& content = tinst.getOps ();
	  auto valT = content.op1->getType();
          auto resT = content.res->getType();
	  if (valT != resT) {
	    mess << MustBeSameType {inst,content.op1,content.res};
            return false;
          }
          return true;
        }

        else if constexpr (i == InstructionCode::Ret) {
	  auto& content = tinst.getOps ();
	  if (tt != content.value->getType()) {
            mess << MustBeGivenType {inst,content.value,tt};
	    
            return false;
          }
          return true;
        }

        else if constexpr (i == InstructionCode::RetVoid) {
          if (tt->getTypeID() != MiniMC::Model::TypeID::Void) {
            MiniMC::Support::Localiser must_be_same_type("Return type of function with '%1%' must be void  ");
            mess << MiniMC::Support::TError {must_be_same_type.format(MiniMC::Model::InstructionCode::RetVoid)};
            return false;
          }
          return true;
        }

        else if constexpr (i == InstructionCode::NonDet) {
	  auto& content = tinst.getOps ();
	  auto type = content.res->getType();
          if (!type->isInteger ()) {
            mess << MustBeInteger {inst,content.res};
	    return false;
          }

          return true;
        }

        else if constexpr (i == InstructionCode::Assert ||
                           i == InstructionCode::Assume ||
                           i == InstructionCode::NegAssume) {
          auto& content = tinst.getOps ();
          auto type = content.expr->getType();
          if (type->getTypeID() != MiniMC::Model::TypeID::Bool ) {
	    mess << MustBeGivenTypeID {inst,content.expr,MiniMC::Model::TypeID::Bool};
            return false;
          }
          return true;
        }

	else if constexpr (i == InstructionCode::ExtractValue ||
                           i == InstructionCode::InsertValue
			   ) {
          auto& content = tinst.getOps ();
	  if (!content.offset->getType ()->isInteger ()) {
	    mess << MustBeInteger {inst,content.offset};
            return false;
          }

	  if (!content.aggregate->getType ()->isAggregate ()) {
	    mess << MustBeGivenTypeID {inst,content.aggregate, MiniMC::Model::TypeID::Array};
            return false;
          }

	  if constexpr (i == InstructionCode::InsertValue) {
	    auto& content = tinst.getOps ();
	    if (!content.res->getType ()->isAggregate ()) {
	      mess << MustBeGivenTypeID {inst,content.res, MiniMC::Model::TypeID::Array};
            return false;
	    }
	  }
	  
	  return true;
        }
        
        else if constexpr (i == InstructionCode::BitCast) {
          MiniMC::Support::Localiser warning("TypeCheck not fully implemented for '%1%'");
	  mess << MiniMC::Support::TWarning {warning.format(i)};
          return true;
        }

        else if constexpr (i == InstructionCode::Uniform) {
          auto& content = tinst.getOps ();
          if (!MiniMC::Model::hasSameTypeID({content.res->getType(),
                                             content.max->getType(),
                                             content.min->getType()})) {
	    mess << MustBeGivenType {inst,content.min,content.res->getType ()};
	    mess << MustBeGivenType {inst,content.max,content.res->getType ()};
	    
	    return false;
          }

          if (!content.res->getType()->isInteger ()) {
	    mess << MustBeInteger {inst,content.res};
            return false;
          }

          return true;
        }

        else {
          []<bool b = false>() { static_assert(b && "cannot type check instruction"); }
          ();
        }
      }

      bool TypeChecker::Check(MiniMC::Model::Function& F, MiniMC::Support::Messager mess) {
	bool res{true};
	for (auto& E : F.getCFA().getEdges()) {
	  const auto& instrkeeper = E->getInstructions ();
	  if (instrkeeper) {
	    for (auto& I : instrkeeper) {
	      if (!I.visit ([this,&F,&I,&mess](auto& instr) {
		return doCheck (instr,I,F.getReturnType (),prgm,mess);
	      })) {
		res = false;
	      }
	    }
	  }
	}
	return res;
      }
      
      bool TypeChecker::Check(MiniMC::Support::Messager mess) {
        bool res = true;
        for (auto& F : prgm.getFunctions()) {
	  res = res && Check (*F,mess);
        }
        return res;
      }
    } // namespace Checkers
  }   // namespace Model
} // namespace MiniMC
