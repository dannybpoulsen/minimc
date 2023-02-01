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
      struct StateQuery : public MiniMC::CPA::StateQuery {
        CommonState_ptr makeInitialState(const InitialiseDescr&) override;
      };

      struct Transferer : public MiniMC::CPA::Transferer {
	Transferer (const MiniMC::Model::Program& prgm) : prgm(prgm) {} 
        CommonState_ptr doTransfer(const CommonState& s, const MiniMC::Model::Edge&, proc_id) override;
      private:
	const MiniMC::Model::Program& prgm;
      };

      struct Joiner : public MiniMC::CPA::Joiner {
        /** 
	 * The Location tracking CPA can only join if the two states has equal hash value.  
	 *
	 * @return 
	 */
        CommonState_ptr doJoin(const CommonState& l, const CommonState& r) override {
          if (std::hash<MiniMC::CPA::CommonState>{}(l) == std::hash<MiniMC::CPA::CommonState>{}(r))
            return l.shared_from_this ();
          return nullptr;
        }

        /** 
	 *  \p l covers \p r if their hash values are the same 
	 */
        bool covers(const CommonState& l, const CommonState& r) override {
          return std::hash<MiniMC::CPA::CommonState>{}(l) == std::hash<MiniMC::CPA::CommonState>{}(r);
        }
      };

      using CPA = CPADef<
          StateQuery,
          Transferer,
          Joiner>;

    } // namespace Location
  } // namespace CPA
} // namespace MiniMC



#endif
