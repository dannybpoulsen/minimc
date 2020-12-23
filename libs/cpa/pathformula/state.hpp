#ifndef _pathSTATE__
#define _pathSTATE__

#include <memory>
#include "util/ssamap.hpp"
#include "cpa/interface.hpp"
#include "smt/context.hpp"

namespace MiniMC {
  namespace CPA {
    namespace PathFormula {
	  class State : public MiniMC::CPA::State
      {
	  public:
		State (const MiniMC::Util::SSAMap& map, const SMTLib::Context_ptr& context, const SMTLib::Term_ptr& path) : context(context),map(map),pathformula(path) {}
		State (const State& oth) = default;
		virtual std::ostream& output (std::ostream& os) const {return os << map << "\nPathformula:" << *pathformula;}
		MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t seed = 0) const override {return reinterpret_cast<MiniMC::Hash::hash_t> (this);}
		virtual std::shared_ptr<MiniMC::CPA::State> copy () const {return std::make_shared<State> (*this);}
		virtual bool need2Store () const {return false;}
		auto& getSSAMap () {return map;}
		auto& getSSAMap () const {return map;}
		auto& getContext () const {return context;}
		auto& getContext ()  {return context;}
		auto& getPathFormula () {return pathformula;}
		const auto& getPathFormula () const {return pathformula;}
		
	  private:
		SMTLib::Context_ptr context;
		MiniMC::Util::SSAMap map;
		SMTLib::Term_ptr pathformula;
	  };
	}
  }
}


#endif
