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
    
    class EdgeBuilder {
    public:
      EdgeBuilder (MiniMC::Model::CFA& cfa,
		   const MiniMC::Model::Location_ptr& from,
		   const MiniMC::Model::Location_ptr& to,
		   MiniMC::Model::Frame& frame,
		   bool isPhi = false
		   ) :
	cfa(cfa),
	from(from),
	to(to),
	frame(frame),
	isPhi(isPhi)
      {
      }

      ~EdgeBuilder () {
	if (stream || lastEdge == nullptr) {
	  cfa.makeEdge (from,to,std::move(stream),isPhi);
	}
	else {
	  auto stream = lastEdge->getInstructions();
	  cfa.makeEdge (lastEdge->getFrom (),to,std::move(stream),lastEdge->isPhi ());
	  cfa.deleteEdge (lastEdge.get ());

	}
      }
      
      template<MiniMC::Model::InstructionCode code,class... Args>
      EdgeBuilder& addInstr (Args... args)  {
	if (!isPhi) {
	  if constexpr (code == MiniMC::Model::InstructionCode::Call ||
			code == MiniMC::Model::InstructionCode::NonDet ||
			code == MiniMC::Model::InstructionCode::Uniform ||	  
			code ==MiniMC::Model::InstructionCode::Assert
			){
	    auto instr = MiniMC::Model::Instruction::make<code> (args...);
	    break_edge ();
	    stream.add (instr);
	    break_edge ();
	  }
	
	  else {
	    auto instr = MiniMC::Model::Instruction::make<code> (args...);
	    stream.add (instr);
	  }
	}
	else {
	  if constexpr (code!=MiniMC::Model::InstructionCode::Assign) {
	    throw MiniMC::Support::Exception ("Phi-edges can only have assigns");
	  }
	  auto instr = MiniMC::Model::Instruction::make<code> (args...);
	  stream.add (instr);
	  return *this;
      
	}
	
	
	return *this;
      }

      
      void setPhi () {
	if (!isPhi) {
	  break_edge ();
	  isPhi = true;
	}
      }
      
      void clearPhi () {
	if (isPhi) {
	  break_edge ();
	  isPhi = false;
	}
      }
      
    private:
      void break_edge () {
	if (stream) {
	  auto nto = cfa.makeLocation (frame.makeFresh (), from->getInfo ());
	  lastEdge = cfa.makeEdge (from,nto,std::move(stream),isPhi);
	  from = nto;;
	  stream.clear ();
	}
      }
      
      MiniMC::Model::CFA& cfa;
      MiniMC::Model::Location_ptr from;
      MiniMC::Model::Location_ptr to;
      MiniMC::Model::InstructionStream stream;
      MiniMC::Model::Frame frame;
      MiniMC::Model::Edge_ptr lastEdge{nullptr};
      bool isPhi;
    };
    
    
    
  }
}

#endif
