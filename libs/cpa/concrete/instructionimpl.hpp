#ifndef _INSTRUCTION_IMPL
#define _INSTRUCTION_IMPL

#include "model/instructions.hpp"


#include "model/variables.hpp"
#include "util/array.hpp"
#include "support/types.hpp"

#include "support/random.hpp"
#include "except.hpp"
#include "tacimpl.hpp"
#include "cmpimpl.hpp"
#include "castimpl.hpp"
#include "heap.hpp"


namespace MiniMC {
  namespace CPA {
    namespace Concrete {

	  using VariableLookup = MiniMC::Model::VariableMap<MiniMC::Util::Array>;
	  
	  struct GlobalLocalVariableLookup{
		VariableLookup* global;
		VariableLookup* local;
		Heap* heap;
		auto LookUp (const MiniMC::Model::Variable_ptr& v) const  {
		  if (v->isGlobal ()) {
			return global->at (v);
		  }
		  else
			return local->at (v);
		}
		
		void set (const MiniMC::Model::Variable_ptr& v, const MiniMC::Util::Array& arr) {
		  assert (v->getType ()->getSize () == arr.getSize ());
		  if (v->isGlobal ()) {
			(*global)[v] = arr;
		  }
		  else  {
			assert(arr.getSize () == v->getType ()->getSize ());
			
			(*local)[v] = arr;
		  }
		}

		const MiniMC::Util::Array evaluate (const MiniMC::Model::Value_ptr& v) const {
		  if (v->isVariable ()) {
			auto var =  std::static_pointer_cast<MiniMC::Model::Variable> (v);
			auto arr = LookUp (var);
			assert(arr.getSize () == v->getType ()->getSize ());
			return arr;
		  }
		  else {
		    auto constant = std::static_pointer_cast<MiniMC::Model::Constant> (v);
		    if (constant -> isUndef ()) {
		      throw MiniMC::Support::Exception ("No Evaluation of Undef constants available");
			
		    }
		    else if (constant->isNonCompileConstant ()) {
		      throw MiniMC::Support::Exception ("No Evaluation of Noncompile constants available");
		    }
		    else {
		      MiniMC::Util::Array arr(constant->getSize ());
		      arr.set_block (0,constant->getSize (),constant->getData ());
		      return arr;
		    }
		    
		  }
		}
		
	  };
	  
	  struct VMData {
	    GlobalLocalVariableLookup readFrom;
	    GlobalLocalVariableLookup writeTo;
	    void finalise () {}
	  };
	  
	  struct ExecuteInstruction {

		template<MiniMC::Model::InstructionCode opc>
		static void execute (VMData& data,
							 const MiniMC::Model::Instruction& i)  {
		  MiniMC::Model::InstHelper<opc> helper (i);
		  if constexpr (MiniMC::Model::InstructionData<opc>::isTAC) {
			auto& res = helper.getResult ();
			auto& left = helper.getLeftOp ();
			auto& right = helper.getRightOp ();

			auto lval = data.readFrom.evaluate (left);
			auto rval = data.readFrom.evaluate (right);
			data.writeTo .set (std::static_pointer_cast<MiniMC::Model::Variable> (res), Steptacexec<opc> (lval,rval));
			
		  }
		  
		  else if constexpr (MiniMC::Model::InstructionData<opc>::isComparison) {
			auto& res = helper.getResult ();
			auto& left = helper.getLeftOp ();
			auto& right = helper.getRightOp ();
			 
			auto lval = data.readFrom.evaluate (left);
			auto rval = data.readFrom.evaluate (right);
			assert(lval.getSize () == left->getType()->getSize ());
			data.writeTo .set (std::static_pointer_cast<MiniMC::Model::Variable> (res), Stepcmpexec<opc> (lval,rval));
			
		  }

		  else if constexpr (MiniMC::Model::InstructionData<opc>::isPredicate) {
			 auto& left = helper.getLeftOp ();
			 auto& right = helper.getRightOp ();
			 
			 auto lval = data.readFrom.evaluate (left);
			 auto rval = data.readFrom.evaluate (right);
			 Steppredexec<opc> (lval,rval);
			 
		  }
		  
		  else if constexpr (MiniMC::Model::InstructionData<opc>::isCast) {
			auto& res = helper.getResult ();
			auto& left = helper.getCastee ();
			auto lval = data.readFrom.evaluate (left);
			data.writeTo .set (std::static_pointer_cast<MiniMC::Model::Variable> (res), Stepcastexec1<opc> (lval,res->getType()->getSize()));
			
		  }

		  else if constexpr (opc == MiniMC::Model::InstructionCode::Assign) {
			auto& res = helper.getResult ();
			auto& left = helper.getValue ();
			auto lval = data.readFrom.evaluate (left);
			data.writeTo .set (std::static_pointer_cast<MiniMC::Model::Variable> (res), lval);
		  }

		  else if constexpr (opc == MiniMC::Model::InstructionCode::Skip) {
			
		  }
		   
		  else if constexpr (opc == MiniMC::Model::InstructionCode::Assume) {
			auto& val = helper.getAssert ();
			auto lval = data.readFrom.evaluate (val);
			if (!lval.template read<MiniMC::uint8_t> ())
			  throw MiniMC::Support::AssumeViolated ();
		  }

		  else if constexpr (opc == MiniMC::Model::InstructionCode::Uniform) {
			auto& res = helper.getResult ();
			auto& min = helper.getMin ();
			auto& max = helper.getMax ();
			
			auto lmin = data.readFrom.evaluate (min);
			auto lmax = data.readFrom.evaluate (max);

			
			auto mod = [&]<typename T> () {
			  MiniMC::Util::Array arrres (sizeof (T));
			  arrres.template set<T> (0,MiniMC::Support::RandomNumber{}.uniform (lmin.template read<T>(), lmax.template read<T>()));;
			  data.writeTo.set (std::static_pointer_cast<MiniMC::Model::Variable> (res),arrres);
			};
			
			switch (res->getType()->getSize ()) {
			case 1:
			  mod.template operator()<MiniMC::uint8_t> ();
			  break;
			case 2:
			  mod.template operator()<MiniMC::uint16_t> ();
			  break;
			case 4:
			  mod.template operator()<MiniMC::uint32_t> ();
			  break;
			case 8:
			  mod.template operator()<MiniMC::uint64_t> ();
			  break;
			}
			
		  }
		  
		  else if constexpr (opc == MiniMC::Model::InstructionCode::Assert) {
			auto& val = helper.getAssert ();
			auto lval = data.readFrom.evaluate (val);
			if (!lval.template read<MiniMC::uint8_t> ())
			  throw MiniMC::Support::AssertViolated ();
		  }

		  else if constexpr (opc == MiniMC::Model::InstructionCode::NegAssume) {
			auto& val = helper.getAssert ();
			auto lval = data.readFrom.evaluate (val);
			if (lval.template read<MiniMC::uint8_t> ())
			  throw MiniMC::Support::AssumeViolated ();
		  }

		  else if constexpr (opc == MiniMC::Model::InstructionCode::Malloc) {
			//Ignore. The space is technically already reserved by FindSpace
		  }

		  else if constexpr (opc == MiniMC::Model::InstructionCode::Free) {
			auto& pointer = helper.getPointer ();
			auto lpointer = data.readFrom.evaluate (pointer);
			data.writeTo.heap->free (lpointer.template read<pointer_t> ());
		  }
		  
		  else if constexpr (opc == MiniMC::Model::InstructionCode::Alloca ||
							 opc == MiniMC::Model::InstructionCode::FindSpace) {
			auto& result = helper.getResult ();
			auto& size = helper.getSize ();
			auto lsize = data.readFrom.evaluate (size);
			MiniMC::pointer_t pointer = data.writeTo.heap->allocate (lsize.template read<MiniMC::uint64_t> (0));
			MiniMC::Util::Array res (sizeof(pointer));
			res.set (0,pointer);
			data.writeTo.set (std::static_pointer_cast<MiniMC::Model::Variable> (result),res);
			
		  }
		  
		  else if constexpr (opc == MiniMC::Model::InstructionCode::ExtendObj) {
			auto& result = helper.getResult ();
			auto& size = helper.getSize ();
			auto lsize = data.readFrom.evaluate (size);
			auto& pointer = helper.getPointer ();
			auto lpointer = data.readFrom.evaluate (pointer);
			
			MiniMC::pointer_t pointer_res = data.writeTo.heap->extend (lpointer.template read<pointer_t> (),
																	   lsize.template read<MiniMC::uint64_t> (0));
			MiniMC::Util::Array res (sizeof(pointer_res));
			res.set (0,pointer_res);
			data.writeTo.set (std::static_pointer_cast<MiniMC::Model::Variable> (result),res);
			
		  }
		  
		  else if constexpr (opc == MiniMC::Model::InstructionCode::Store) {
			auto addr = data.readFrom.evaluate (helper.getAddress ());
			auto value = data.readFrom.evaluate (helper.getValue ());
			data.writeTo.heap->write (value,addr.template read<MiniMC::pointer_t> ());
			
		  }

		  else if constexpr (opc == MiniMC::Model::InstructionCode::Load) {
			auto& result = helper.getResult ();
			MiniMC::Util::Array res (result->getType ()->getSize());
			auto addr = data.readFrom.evaluate (helper.getAddress ());
			data.readFrom.heap->read (res,addr.template read<MiniMC::pointer_t> ());
			data.writeTo.set (std::static_pointer_cast<MiniMC::Model::Variable> (result),res);
			
		  }

		  else if constexpr (opc == MiniMC::Model::InstructionCode::PtrAdd) {
			auto& result = helper.getResult ();
			auto addr = data.readFrom.evaluate (helper.getAddress ());
			auto value = data.readFrom.evaluate (helper.getValue ());
			auto skip = data.readFrom.evaluate (helper.getSkipSize ());

			MiniMC::uint64_t jump = value.template read<MiniMC::uint64_t> ()* skip.template read<MiniMC::uint64_t> ();
			MiniMC::pointer_t resptr = MiniMC::Support::ptradd (addr.template read<MiniMC::pointer_t> (),jump);

			MiniMC::Util::Array res (sizeof(MiniMC::pointer_t));
			res.set (0,resptr);
			
			
			data.writeTo.set (std::static_pointer_cast<MiniMC::Model::Variable> (result),res);
			
		  }
		  
		  else {
			throw NotImplemented<opc> ();
		  }
		   
		}
	  };

	}
  }
}

namespace std {
  template<>
  struct hash<MiniMC::CPA::Concrete::VariableLookup > {
	auto operator() (const MiniMC::CPA::Concrete::VariableLookup& arr) {
	  return arr.hash (0);
	}
  };
  
}


#endif
