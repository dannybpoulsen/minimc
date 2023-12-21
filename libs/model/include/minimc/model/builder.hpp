#ifndef _MODEL_BUILDER__
#define _MODEL_BUILDER__

#include "minimc/hash/hashing.hpp"
#include "minimc/model/symbol.hpp"
#include "minimc/model/edge.hpp"
#include "minimc/model/cfg.hpp"
#include "minimc/model/location.hpp"
#include "minimc/model/instructions.hpp"

#include <unordered_map>

namespace MiniMC {
  namespace Model {
    
    template<bool isPhi = false>
    class EdgeBuilder {
    public:
      EdgeBuilder (MiniMC::Model::CFA& cfa,
		   const MiniMC::Model::Location_ptr& from,
		   const MiniMC::Model::Location_ptr& to,
		   MiniMC::Model::Frame& frame
		   ) :
	cfa(cfa),
	from(from),
	to(to),
	frame(frame){
      }

      ~EdgeBuilder () {
	cfa.makeEdge (from,to,std::move(stream),isPhi);
      }

      template<MiniMC::Model::InstructionCode code,class... Args>
      EdgeBuilder& addInstr (Args... args) requires (!isPhi && hasOperands<code>) {
	//typename InstructionData<code>::Content  content(args...);
	auto instr = MiniMC::Model::Instruction::make<code> (args...);
	if constexpr (code == MiniMC::Model::InstructionCode::Call ||
		      code == MiniMC::Model::InstructionCode::NonDet ||
		      code == MiniMC::Model::InstructionCode::Uniform ||	  
		      code ==MiniMC::Model::InstructionCode::Assume ||
		      code ==MiniMC::Model::InstructionCode::NegAssume ||
		      code ==MiniMC::Model::InstructionCode::Assert
		      ){
	  auto nto = cfa.makeLocation (frame.makeFresh (), from->getInfo ());
	  cfa.makeEdge (from,nto,std::move(stream));
	  from = cfa.makeLocation (frame.makeFresh (),from->getInfo ());
	  
	  cfa.makeEdge (nto,from,MiniMC::Model::InstructionStream({instr}));
	  
	  
	}
	
	else {
	  if constexpr (code == MiniMC::Model::InstructionCode::ZExt) {
	    if (instr. template getAs<MiniMC::Model::InstructionCode::ZExt> ().getOps ().op1->getType()->getTypeID () == MiniMC::Model::TypeID::Bool) {
	      instr = MiniMC::Model::Instruction::make<MiniMC::Model::InstructionCode::BoolZExt> (args...);
	    }
	  }

	  if constexpr (code == MiniMC::Model::InstructionCode::SExt) {
	    if (instr.template  getAs<MiniMC::Model::InstructionCode::SExt> ().getOps ().op1->getType()->getTypeID () == MiniMC::Model::TypeID::Bool) {
	      instr = MiniMC::Model::Instruction::make<MiniMC::Model::InstructionCode::BoolSExt> (args...);
	    }
	  }
	  
	  stream.add (instr);
	 
	}
	
	
	
	return *this;
      }

      template<MiniMC::Model::InstructionCode code>
      EdgeBuilder& addInstr () requires (!isPhi && !hasOperands<code>) {
	
	auto instr = MiniMC::Model::Instruction::make<code> ();
	stream.add (instr);
	return *this;
      }

      template<MiniMC::Model::InstructionCode code,class... Args>
      EdgeBuilder& addInstr (Args... args) requires (isPhi) {
	static_assert(code==MiniMC::Model::InstructionCode::Assign && "Phi edges can only have assign");
	auto instr = MiniMC::Model::Instruction::make<code> (args...);
	stream.add (instr);
	
	
	return *this;
      }
      
      
    private:
      MiniMC::Model::CFA& cfa;
      MiniMC::Model::Location_ptr from;
      MiniMC::Model::Location_ptr to;
      MiniMC::Model::InstructionStream stream;
      MiniMC::Model::Frame frame;
    };
    
    
    
  }
}

#endif
