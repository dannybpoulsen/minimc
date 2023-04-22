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
      
      struct Transferer : public MiniMC::CPA::TTransfer<CommonState> {
	Transferer (const MiniMC::Model::Program& prgm) : prgm(prgm) {} 
        State_ptr<CommonState> doTransfer(const CommonState& s, const MiniMC::Model::Edge&, proc_id) override;
      private:
	const MiniMC::Model::Program& prgm;
      };

      struct Joiner : public MiniMC::CPA::TJoiner<CommonState> {
        /** 
	 * The Location tracking CPA can only join if the two states has equal hash value.  
	 *
	 * @return 
	 */
        State_ptr<CommonState> doJoin(const CommonState& l, const CommonState& r) override {
          if (std::hash<MiniMC::CPA::CommonState>{}(l) == std::hash<MiniMC::CPA::CommonState>{}(r))
            return l.shared_from_this ();
          return nullptr;
        }

      };

      struct CPA : public ICPA<CommonState> {
	State_ptr<CommonState> makeInitialState(const InitialiseDescr&) override;
	TTransferer_ptr<CommonState> makeTransfer(const MiniMC::Model::Program& prgm ) const override {return std::make_shared<Transferer> (prgm);}
	TJoiner_ptr<CommonState> makeJoin( ) const override {return std::make_shared<Joiner> ();} 
	
      };
      
      
    } // namespace Location
  } // namespace CPA
} // namespace MiniMC



#endif
