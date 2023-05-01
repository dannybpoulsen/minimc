/**
 * @file   location.hpp
 * @date   Mon Apr 20 18:20:00 2020
 * 
 *  This file contains the definition of a Location tracking CPA
 * 
 * 
 */
#ifndef _LOCATION__
#define _LOCATION__

#include "cpa/interface.hpp"
#include "model/cfg.hpp"

namespace MiniMC {
  namespace CPA {
    namespace Location {
      
      struct Transferer : public MiniMC::CPA::TTransfer<CFAState> {
	Transferer (const MiniMC::Model::Program& prgm) : prgm(prgm) {} 
        State_ptr<CFAState> doTransfer(const CFAState& s, const Transition&) override;
      private:
	const MiniMC::Model::Program& prgm;
      };

      struct Joiner : public MiniMC::CPA::TJoiner<CFAState> {
        /** 
	 * The Location tracking CPA can only join if the two states has equal hash value.  
	 *
	 * @return 
	 */
        State_ptr<CFAState> doJoin(const CFAState& l, const CFAState& r) override {
          if (std::hash<MiniMC::CPA::CFAState>{}(l) == std::hash<MiniMC::CPA::CFAState>{}(r))
            return l.shared_from_this ();
          return nullptr;
        }

      };

      struct CPA : public ICPA<CFAState> {
	State_ptr<CFAState> makeInitialState(const InitialiseDescr&) override;
	TTransferer_ptr<CFAState> makeTransfer(const MiniMC::Model::Program& prgm ) const override {return std::make_shared<Transferer> (prgm);}
	TJoiner_ptr<CFAState> makeJoin( ) const override {return std::make_shared<Joiner> ();} 
	
      };
      
      
    } // namespace Location
  } // namespace CPA
} // namespace MiniMC



#endif
