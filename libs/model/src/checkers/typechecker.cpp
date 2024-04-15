#include "minimc/model/checkers/typechecker.hpp"

#include "minimc/model/cfg.hpp"
#include "minimc/model/instructions.hpp"
#include "minimc/model/valuevisitor.hpp"
#include "minimc/model/variables.hpp"
#include "minimc/support/feedback.hpp"
#include "minimc/support/localisation.hpp"

#include <iostream>

namespace MiniMC {
  namespace Model {
    namespace Checkers {

      
      
      class InstrTypeError : public MiniMC::Support::ErrorMessage {
      public:
	InstrTypeError (MiniMC::Model::Instruction instr) : instr(std::move(instr)) {}
	
      protected:
	auto& getInstr () const {return instr;}
      private:
	std::string mess;
	MiniMC::Model::Instruction instr;
      };

      class ExprTypeError : public MiniMC::Support::ErrorMessage {
      public:
	
      };

      class MustBeSameTypeExpr : public ExprTypeError{
      public:
	MustBeSameTypeExpr (const MiniMC::Model::Value& l1, const MiniMC::Model::Value& l2 )  {
	  message = MiniMC::Support::Localiser {"'%1%' and '%2%' must have same type."}.format(l1,l2);
	
	}
	
	virtual std::ostream& to_string (std::ostream& os) const {
	  return os << message;
	}
	
      private:
	std::string message;
	
      };
      
      
      class MustBeSameType : public InstrTypeError{
      public:
	MustBeSameType (MiniMC::Model::Instruction instr, MiniMC::Model::Value_ptr l1, MiniMC::Model::Value_ptr l2 ) : InstrTypeError (std::move(instr)),l1(l1),l2(l2) {}
	virtual std::ostream& to_string (std::ostream& os) const {
	  return os << loc.format (getInstr (),*l1,*l2);
	}
	
      private:
	MiniMC::Support::Localiser loc{"For Instruction '%1%': '%2%' and '%3%' must have same type."};
	MiniMC::Model::Value_ptr l1;
	MiniMC::Model::Value_ptr l2;
      };
      
      class MustBeGivenTypeID : public InstrTypeError{
      public:
	MustBeGivenTypeID (MiniMC::Model::Instruction instr, MiniMC::Model::Value_ptr l1, MiniMC::Model::TypeID type ) : InstrTypeError (std::move(instr)),l1(l1),type(type) {}
	virtual std::ostream& to_string (std::ostream& os) const {
	  return os << loc.format (getInstr (),*l1,type);
	}
	
      private:
	MiniMC::Support::Localiser loc{"For Instruction '%1%': '%2%' must be of type '%3%'."};
	MiniMC::Model::Value_ptr l1;
	MiniMC::Model::TypeID type;
      };

      class MustBeGivenType : public InstrTypeError{
      public:
	MustBeGivenType (MiniMC::Model::Instruction instr, MiniMC::Model::Value_ptr l1, MiniMC::Model::Type_ptr type ) : InstrTypeError (std::move(instr)),l1(l1),type(type) {}
	virtual std::ostream& to_string (std::ostream& os) const {
	  return os << loc.format (getInstr (),*l1,*type);
	}
	
      private:
	MiniMC::Support::Localiser loc{"For Instruction '%1%': '%2%' must be of type '%3%'."};
	MiniMC::Model::Value_ptr l1;
	MiniMC::Model::Type_ptr type;
      };
      
      class MustBeInteger : public InstrTypeError{
      public:
	MustBeInteger (MiniMC::Model::Instruction instr, MiniMC::Model::Value_ptr l1 ) : InstrTypeError (std::move(instr)),l1(l1) {}
	virtual std::ostream& to_string (std::ostream& os) const {
	  return os << loc.format (getInstr (),*l1);
	}
	
      private:
	MiniMC::Support::Localiser loc{"For Instruction '%1%': '%2%' must be an integer type."};
	MiniMC::Model::Value_ptr l1;
      };

      class MustBeLarger : public InstrTypeError{
      public:
	MustBeLarger (MiniMC::Model::Instruction instr, MiniMC::Model::Type_ptr t1,MiniMC::Model::Type_ptr t2 ) : InstrTypeError (std::move(instr)),t1(t1),t2(t2) {}
	virtual std::ostream& to_string (std::ostream& os) const {
	  return os << loc.format (getInstr (),*t1,*t2);
	}
	
      private:
	MiniMC::Support::Localiser loc{"For Instruction '%1%': '%2%' must be of larger size thant %3%"};
	MiniMC::Model::Type_ptr t1;
	MiniMC::Model::Type_ptr t2;
	
      };

      class MustBeConstant : public InstrTypeError {
      public:
	MustBeConstant (MiniMC::Model::Instruction instr, MiniMC::Model::Value_ptr t1 ) : InstrTypeError (std::move(instr)),val(t1) {}
	virtual std::ostream& to_string (std::ostream& os) const {
	  return os << loc.format (getInstr (),*val);
	}
      private:
	MiniMC::Support::Localiser loc{"For Instruction '%1%': '%2%' must be constant"};
	
	Value_ptr val;
      };
      
      template<class T>
      MiniMC::Model::Type_ptr  TypeChecker::operator() (T& expr) const requires is_bin_arith<T> {
	auto lty = CheckType(expr.getLeft());
	auto rty = CheckType(expr.getRight());
	MiniMC::Model::Type_ptr res = nullptr;;
	if (lty == rty) {
	  res = lty;
	}
	else {
	  messager << MustBeSameTypeExpr{expr.getLeft(),expr.getRight()}
	  ;
	}
	expr.setType (res);
	return res;
	  
      }

      template<class T>
      MiniMC::Model::Type_ptr  TypeChecker::operator() (T& expr) const requires is_bin_cmp<T> {
	auto lty = CheckType(expr.getLeft());
	auto rty = CheckType(expr.getRight());
	MiniMC::Model::Type_ptr res = nullptr;;
	if (lty == rty) {
	  res = prgm.getTypeFactory().makeBoolType ();
	}
	else {
	  messager << MustBeSameTypeExpr{expr.getLeft(),expr.getRight()}
	  ;
	}
	expr.setType (res);
	return res;
	  
      }
	
      
      
      template <class Inst>
      bool TypeChecker::doCheck(const Inst& tinst, const MiniMC::Model::Instruction& inst, const MiniMC::Model::Type_ptr& tt, MiniMC::Model::Program& prgm) {
	constexpr auto i = tinst.getOpcode ();
	if constexpr (InstructionData<i>::isTAC ) {
	  auto& content = tinst.getOps ();
	  auto resType = CheckType(*content.res);
          auto lType = CheckType(*content.op1);
          auto rType = CheckType(*content.op2);
          if (resType != lType ||
              lType != rType ||
              rType != resType) {
            messager << MustBeSameType {inst,content.op1,content.op2};
            return false;
          }
          return true;
        } 

        else if constexpr (InstructionData<i>::isUnary) {
	  auto& content = tinst.getOps ();
	  if constexpr (i == MiniMC::Model::InstructionCode::Not) {
            auto resType = CheckType(*content.res);
            auto lType = CheckType(*content.op1);
            if (resType != lType) {
              messager << MustBeSameType {inst,content.res,content.op1};
              return false;
            }
            return true;
          }
	  if constexpr (i == MiniMC::Model::InstructionCode::LogNot) {
            auto resType = CheckType(*content.res);
            auto lType = CheckType(*content.op1);
            if (resType != lType) {
              messager << MustBeSameType {inst,content.res,content.op1};
              return false;
            }
	    if (resType->getTypeID () != MiniMC::Model::TypeID::Bool) {
	      messager << MustBeGivenTypeID (inst,content.res,MiniMC::Model::TypeID::Bool);
	      return false;
	    }
            return true;
          }
        }

        else if constexpr (InstructionData<i>::isComparison) {
	  auto& content = tinst.getOps ();
	  
	  
          auto resType = content.res->getType();
          auto lType = CheckType(*content.op1);
          auto rType = CheckType(*content.op2);
          if (lType != rType) {
            messager << MustBeSameType {inst,content.op1,content.op2};
            return false;
          }
	  
	  else if (resType->getTypeID() != MiniMC::Model::TypeID::Bool) {
            messager << MustBeGivenTypeID (inst,content.res,MiniMC::Model::TypeID::Bool);
            return false;
          }

          return true;
        }

        else if constexpr (i == InstructionCode::Trunc) {
	  auto& content = tinst.getOps ();
          auto ftype = CheckType(*content.op1);
          auto ttype = CheckType(*content.res);

	  if (!ftype->isInteger ()) {
	    messager << MustBeInteger (inst,content.op1);
	    return false;
	  }
	  if (!ttype->isInteger ()) {
	    messager << MustBeInteger (inst,content.op1);
	    return false;
	  }
	  if (ftype->getSize() <= ttype->getSize()) {
	    messager << MustBeLarger {inst,ttype,ftype};
	    return false;
          }

          return true;
        }

        else if constexpr (i == InstructionCode::IntToBool) {
	  auto& content = tinst.getOps ();
	  auto ftype = CheckType(*content.op1);
          auto ttype = CheckType(*content.res);

	  if (!ftype->isInteger ()) {
	    messager << MustBeInteger {inst,content.op1};
	    return false;
	  }

	  if (ttype->getTypeID() != MiniMC::Model::TypeID::Bool) {
	    messager << MustBeGivenTypeID {inst,content.res,MiniMC::Model::TypeID::Bool};
	    return false;
	  }
	  
          return true;
        }

        else if constexpr (i == InstructionCode::SExt ||
                           i == InstructionCode::ZExt) {
	  auto& content = tinst.getOps ();
	  auto ftype = CheckType(*content.op1);
          auto ttype = CheckType(*content.res);

          if (!ftype->isInteger ()) {
	    messager << MustBeInteger (inst,content.op1);
	    return false;
	  }
	  if (!ttype->isInteger ()) {
	    messager << MustBeInteger (inst,content.op1);
	    return false;
	  }
	  if (ftype->getSize() >= ttype->getSize()) {
	    messager << MustBeLarger {inst,ttype,ftype};
	    return false;
          }
          return true;
        }

        else if constexpr (i == InstructionCode::BoolSExt ||
                           i == InstructionCode::BoolZExt) {
	  auto& content = tinst.getOps ();
          MiniMC::Support::Localiser must_be_integer("'%1%' can only be applied from boolean types to  integer types. ");
          MiniMC::Support::Localiser must_be_smaller("From type must be smaller that to type for '%1%'");

          auto ftype = CheckType(*content.op1);
          auto ttype = CheckType(*content.res);

	  if (ftype->getTypeID () != MiniMC::Model::TypeID::Bool) {
	    messager << MustBeGivenTypeID (inst,content.op1,MiniMC::Model::TypeID::Bool);
	    return false;
	  }
	  
          if (!ttype->isInteger () ) {
	    messager << MustBeInteger {inst,content.res};;
            return false;
          } 

          return true;
        }

        else if constexpr (i == InstructionCode::IntToPtr) {
	  auto& content = tinst.getOps ();
	  MiniMC::Support::Localiser must_be_integer("'%1%' can only be applied to integer types. ");
          MiniMC::Support::Localiser must_be_pointer("Return type has to be pointer for '%1%'");

          auto ftype = CheckType(*content.op1);
          auto ttype = CheckType(*content.res);

          if (!ftype->isInteger ()) {
	    messager << MustBeInteger {inst,content.op1};
            return false;
          }

          else if (ttype->getTypeID() != MiniMC::Model::TypeID::Pointer) {
	    messager << MustBeGivenTypeID (inst,content.res,MiniMC::Model::TypeID::Pointer);
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
          auto ptr = CheckType(*content.ptr);
          auto skip = CheckType(*content.skipsize);
          auto value = CheckType(*content.nbSkips);
          auto result = CheckType(*content.res);
	  
          if (result->getTypeID() != MiniMC::Model::TypeID::Pointer &&
	      result->getTypeID() != MiniMC::Model::TypeID::Pointer32
	      ) {
            messager << MustBeGivenTypeID (inst,content.res,MiniMC::Model::TypeID::Pointer);
	    return false;
          }
	  if (!skip->isInteger () ) {
	    messager << MustBeInteger (inst,content.skipsize);
	    return false;
          }
	  if (!value->isInteger ()) {
	    messager << MustBeInteger (inst,content.nbSkips);
	    return false;
          }
	  if (value != skip) {
	    messager << MustBeSameType {inst,content.skipsize,content.nbSkips};
	    
            return false;
          }
	  
          if (ptr->getTypeID() != MiniMC::Model::TypeID::Pointer &&
	      ptr->getTypeID() != MiniMC::Model::TypeID::Pointer32
	      ) {
	    messager << MustBeGivenTypeID (inst,content.ptr,MiniMC::Model::TypeID::Pointer);
	    
	    
            return false;
          }

	  if (ptr->getTypeID () != result->getTypeID ()) {
	    messager << MustBeSameType {inst,content.ptr,content.res};
	    return false;
          
	  }
	  
          return true;
        }

        else if constexpr (i == InstructionCode::PtrToInt) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only be applied to pointer types. ");
          MiniMC::Support::Localiser must_be_integer("Return type has to be integer for '%1%'");

	  auto& content = tinst.getOps ();
          auto ftype = CheckType(*content.op1);
          auto ttype = CheckType(*content.res);
          if (ftype->getTypeID() != MiniMC::Model::TypeID::Pointer &&
 	      ftype->getTypeID() != MiniMC::Model::TypeID::Pointer32
	      ) {
	    messager << MustBeGivenTypeID (inst,content.op1,MiniMC::Model::TypeID::Pointer);
            return false;
          }

          else if (!ttype->isInteger ()) {
	    messager << MustBeInteger (inst,content.res);
            return false;
          }

          return true;
        }
        
        else if constexpr (i == InstructionCode::Store) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only store to pointer types. ");

	  auto& content = tinst.getOps ();
          auto addr = CheckType(*content.addr);
          if (addr->getTypeID() != MiniMC::Model::TypeID::Pointer &&
	      addr->getTypeID() != MiniMC::Model::TypeID::Pointer32
	      ) {
	    messager << MustBeGivenTypeID (inst,content.addr,MiniMC::Model::TypeID::Pointer);
            return false;
          }

          return true;
        }
	
	
        else if constexpr (i == InstructionCode::Load) {
          MiniMC::Support::Localiser must_be_pointer("'%1%' can only load from pointer types. ");
          MiniMC::Support::Localiser must_be_integer_or_pointer("'%1%' can only load integers or pointers ");

	  auto& content = tinst.getOps ();
          auto addr = CheckType(*content.addr);
          if (addr->getTypeID() != MiniMC::Model::TypeID::Pointer &&
	      addr->getTypeID() != MiniMC::Model::TypeID::Pointer32) {
	    messager << MustBeGivenTypeID (inst,content.addr,MiniMC::Model::TypeID::Pointer);
            
	    
            return false;
          }

          if (!(content.res->getType()->isInteger () ||
		content.res->getType ()->getTypeID () == MiniMC::Model::TypeID::Pointer ||
		content.res->getType ()->getTypeID () == MiniMC::Model::TypeID::Pointer32
		)
	      ){
	    messager << MustBeInteger (inst,content.res);
            
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
	  auto func = MiniMC::Model::visitValue<MiniMC::Model::Function_ptr>(
						
						MiniMC::Model::Overload{
						  [&prgm,&function_not_exists,this](const MiniMC::Model::Pointer& val) -> Function_ptr {
						    auto ptr = val.getValue ();
						    if (prgm.functionExists (ptr.base))
						      return prgm.getFunction (ptr.base);
						    else {
						      messager << MiniMC::Support::TError<std::string> {function_not_exists.format(MiniMC::Model::getFunctionId(ptr))};
						      return nullptr;
						    }
						  },
						    [&prgm,this,&function_not_exists](const MiniMC::Model::Pointer32& val) -> Function_ptr {
						      auto ptr = val.getValue ();
						      if (prgm.functionExists (ptr.base))
							return prgm.getFunction (ptr.base);
						      else {
							messager << MiniMC::Support::TError<std::string> {function_not_exists.format(ptr.base)};
							return nullptr;
						      }
						    },
						    [&prgm](const MiniMC::Model::SymbolicConstant& sc) -> Function_ptr {
						      return prgm.getFunction (sc.getValue ());
						    },
						    [&fun,&inst,this](const auto&) -> Function_ptr {
						      messager << MustBeConstant {inst,fun};
						      return nullptr;
						    }
						    },
						*fun
						);
	    
	  
	  if (!func) {
	    return false;
	  }
	  
	  MiniMC::Support::Localiser inconsistent_parameters("Inconsistent number of parameters between call and function prototype '%1%'");
	  if (!func->isVarArgs ()) {
	    if (func->getParameters().size() != content.params.size() ) {
	      messager << MiniMC::Support::TError<std::string> {inconsistent_parameters.format (func->getSymbol ().getName ())};
	      return false;
	    }
	    
	    auto nbParams = content.params.size();
	    auto it = func->getParameters().begin();
	    
	    for (size_t j = 0; j < nbParams; j++, ++it) {
	      auto form_type = CheckType(*(*it));
	      auto act_type = CheckType(*content.params.at(j));
	      if (form_type != act_type) {
		messager << MustBeGivenType {inst,*it,act_type};

		return false;
	      }
	    }
	  }
	  else {
	    messager << MiniMC::Support::TWarning<std::string> {function_is_var_args.format (func->getSymbol ().getName ())};
	  }
	  if (content.res) {
	    auto resType = CheckType(*content.res);
	    if (resType != func->getReturnType()) {
	      messager << MiniMC::Support::TError {inconsistent_parameters.format (func->getSymbol ().getName ())};
	      return false;
	    }
	  }
	
	return true;
	}

        else if constexpr (i == InstructionCode::Assign) {
	  auto& content = tinst.getOps ();
	  auto valT = CheckType(*content.op1);
          auto resT = CheckType(*content.res);
	  if (valT != resT) {
	    messager << MustBeSameType {inst,content.op1,content.res};
            return false;
          }
          return true;
        }

        else if constexpr (i == InstructionCode::Ret) {
	  auto& content = tinst.getOps ();
	  if (tt != content.value->getType()) {
            messager << MustBeGivenType {inst,content.value,tt};
	    
            return false;
          }
          return true;
        }

        else if constexpr (i == InstructionCode::RetVoid) {
          if (tt->getTypeID() != MiniMC::Model::TypeID::Void) {
            MiniMC::Support::Localiser must_be_same_type("Return type of function with '%1%' must be void  ");
            messager << MiniMC::Support::TError {must_be_same_type.format(MiniMC::Model::InstructionCode::RetVoid)};
            return false;
          }
          return true;
        }

        else if constexpr (i == InstructionCode::NonDet) {
	  auto& content = tinst.getOps ();
	  auto type = CheckType(*content.res);
          if (!type->isInteger ()) {
            messager << MustBeInteger {inst,content.res};
	    return false;
          }

          return true;
        }

        else if constexpr (i == InstructionCode::Assert ||
                           i == InstructionCode::Assume ||
                           i == InstructionCode::NegAssume) {
          auto& content = tinst.getOps ();
          auto type = CheckType(*content.expr);
          if (type->getTypeID() != MiniMC::Model::TypeID::Bool ) {
	    messager << MustBeGivenTypeID {inst,content.expr,MiniMC::Model::TypeID::Bool};
            return false;
          }
          return true;
        }

	else if constexpr (i == InstructionCode::ExtractValue ||
                           i == InstructionCode::InsertValue
			   ) {
          auto& content = tinst.getOps ();
	  if (!content.offset->getType ()->isInteger ()) {
	    messager << MustBeInteger {inst,content.offset};
            return false;
          }

	  if (!content.aggregate->getType ()->isAggregate ()) {
	    messager << MustBeGivenTypeID {inst,content.aggregate, MiniMC::Model::TypeID::Aggregate};
            return false;
          }

	  if constexpr (i == InstructionCode::InsertValue) {
	    auto& content = tinst.getOps ();
	    if (!CheckType(*content.res)->isAggregate ()) {
	      messager << MustBeGivenTypeID {inst,content.res, MiniMC::Model::TypeID::Aggregate};
            return false;
	    }
	  }
	  
	  return true;
        }
        
        else if constexpr (i == InstructionCode::BitCast) {
          MiniMC::Support::Localiser warning("TypeCheck not fully implemented for '%1%'");
	  messager << MiniMC::Support::TWarning {warning.format(i)};
          return true;
        }

        else if constexpr (i == InstructionCode::Uniform) {
          auto& content = tinst.getOps ();
          if (!MiniMC::Model::hasSameTypeID({CheckType(*content.res),
		CheckType(*content.max),
		CheckType(*content.min)})) {
	    messager << MustBeGivenType {inst,content.min,content.res->getType ()};
	    messager << MustBeGivenType {inst,content.max,content.res->getType ()};
	    
	    return false;
          }

          if (!content.res->getType()->isInteger ()) {
	    messager << MustBeInteger {inst,content.res};
            return false;
          }

          return true;
        }

        else {
          []<bool b = false>() { static_assert(b && "cannot type check instruction"); }
          ();
        }
      }

      bool TypeChecker::Check(MiniMC::Model::Function& F) {
	bool res{true};
	for (auto& E : F.getCFA().getEdges()) {
	  const auto& instrkeeper = E->getInstructions ();
	  if (instrkeeper) {
	    for (auto& I : instrkeeper) {
	      if (!I.visit ([this,&F,&I](auto& instr) {
		return doCheck (instr,I,F.getReturnType (),prgm);
	      })) {
		res = false;
	      }
	    }
	  }
	}
	return res;
      }
      
      bool TypeChecker::Check() {
        bool res = true;
        for (auto& F : prgm.getFunctions()) {
	  res = res && Check (*F);
        }
        return res;
      }
    } // namespace Checkers
  }   // namespace Model
} // namespace MiniMC
