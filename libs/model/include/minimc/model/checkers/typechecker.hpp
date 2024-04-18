/**
 * @file   typechecker.hpp
 * @date   Mon Apr 20 17:02:30 2020
 * 
 * @brief  
 * 
 * 
 */
#ifndef _TYPECHECK__
#define _TYPECHECK__

#include "minimc/model/valuevisitor.hpp"
#include "minimc/model/cfg.hpp"
#include "minimc/support/feedback.hpp"

namespace MiniMC {
  namespace Model {
    namespace Checkers {
      /**
	   * Typecheck the progrm \p prgm.
	   
	   */
      struct TypeChecker  {
        TypeChecker(MiniMC::Model::Program& prgm,MiniMC::Support::Messager messager) : prgm(prgm),messager(messager)  {}
	bool Check();
	bool Check(MiniMC::Model::Function&);
	MiniMC::Model::Type_ptr  CheckType ( MiniMC::Model::Value& v) const  {
	  return MiniMC::Model::visitValue<MiniMC::Model::Type_ptr> (*this, v);
	}

	
	template<class T>
	MiniMC::Model::Type_ptr operator() (T& t) const requires (MiniMC::Model::is_root<T>) {
	  return t.getType ();
	}

	template<class T>
	MiniMC::Model::Type_ptr  operator() (T& ) const requires is_bin_arith<T>;

	template<class T>
	MiniMC::Model::Type_ptr  operator() (T& ) const requires is_bin_cmp<T>;

	MiniMC::Model::Type_ptr  operator() (LogNotExpr& ) const;
	
	
      private:
	template<class Inst>
	bool doCheck(const Inst&, const MiniMC::Model::Instruction&, const MiniMC::Model::Type_ptr&, MiniMC::Model::Program&);


	MiniMC::Model::Program& prgm;
	mutable MiniMC::Support::Messager messager;
      };
      
    } // namespace Checkers
  }   // namespace Model
} // namespace MiniMC

#endif
 
