#ifndef _MODEL_BUILDER__
#define _MODEL_BUILDER__

#include "model/edge.hpp"
#include "model/cfg.hpp"
#include "model/location.hpp"
#include "model/instructions.hpp"


namespace MiniMC {
  namespace Model {
    template<bool isPhi = false>
    class EdgeBuilder {
    public:
      EdgeBuilder (MiniMC::Model::CFA& cfa,
		   const MiniMC::Model::Location_ptr& from,
		   const MiniMC::Model::Location_ptr& to) : cfa(cfa),
						    from(from),
						    to(to) {
	edge = cfa.makeEdge (from,to);
      }

      template<MiniMC::Model::InstructionCode code>
      EdgeBuilder& addInstr (typename MiniMC::Model::InstructionData<code>::Content content) requires (!isPhi) {
	auto instr = MiniMC::Model::createInstruction<code> (content);
	if constexpr (code == MiniMC::Model::InstructionCode::Call ||
		      code == MiniMC::Model::InstructionCode::NonDet ||
		      code == MiniMC::Model::InstructionCode::Uniform ||	  
		      code ==MiniMC::Model::InstructionCode::Assume ||
		      code ==MiniMC::Model::InstructionCode::NegAssume ||
		      code ==MiniMC::Model::InstructionCode::Assert
		      ){
	  auto nto = cfa.makeLocation (from->getInfo ());
	  edge->setTo (nto);
	  from = cfa.makeLocation (from->getInfo ());
	  
	  auto call_edge = cfa.makeEdge (nto,from);
	  call_edge->getInstructions().addInstruction (instr);
	  edge = cfa.makeEdge (from,to);
	  
	  
	}

	else {
	  edge->getInstructions().addInstruction (instr);
	  
	}
	
	
	return *this;
      }

      template<MiniMC::Model::InstructionCode code>
      EdgeBuilder& addInstr (typename MiniMC::Model::InstructionData<code>::Content content) requires (isPhi) {
	static_assert(code==MiniMC::Model::InstructionCode::Assign && "Phi edges can only have assign");
	auto instr = MiniMC::Model::createInstruction<code> (content);
	edge->getInstructions().addInstruction (instr);
	
	
	return *this;
      }
      
      /*EdgeBuilder& operator<< (const MiniMC::Model::Instruction& instr) {
	if constexpr (!isPhi) {
	  switch (instr.getOpcode ()) {
	  case MiniMC::Model::InstructionCode::Call:
	  case MiniMC::Model::InstructionCode::NonDet:
	  case MiniMC::Model::InstructionCode::Uniform:	  
	  case MiniMC::Model::InstructionCode::Assume:
	  case MiniMC::Model::InstructionCode::NegAssume:
	  case MiniMC::Model::InstructionCode::Assert:
	    {
	      auto nto = cfa.makeLocation (from->getInfo ());
	      edge->setTo (nto);
	      from = cfa.makeLocation (from->getInfo ());
	      
	      auto call_edge = cfa.makeEdge (nto,from);
	      call_edge->getInstructions().addInstruction (instr);
	      edge = cfa.makeEdge (from,to);
	      
	    }
	    break;
	    
	    
	  default:
	    edge->getInstructions().addInstruction (instr);
	    
	    break;
	  }
	  return *this;
	}
	
	else {
	  assert(instr.getOpcode () == MiniMC::Model::InstructionCode::Assign);
	  if (instr.getOpcode () == MiniMC::Model::InstructionCode::Assign)
	    edge->getInstructions().addInstruction (instr);
	  
	  else
	    throw MiniMC::Support::Exception ("Phi edges can only have Assign instructions");
	}
      }
      */
      
    private:
      MiniMC::Model::CFA& cfa;
      MiniMC::Model::Location_ptr from;
      MiniMC::Model::Location_ptr to;
      MiniMC::Model::Edge_ptr edge;
    };
  }
}

#endif
