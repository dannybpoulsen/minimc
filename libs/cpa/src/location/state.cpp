#include "cpa/interface.hpp"
#include "cpa/location.hpp"
#include "hash/hashing.hpp"
#include "model/cfg.hpp"
#include "model/valuevisitor.hpp"

namespace MiniMC {
  namespace CPA {
    namespace Location {

      struct LocationState {
        void push(MiniMC::Model::Location* l) {
	  stack.push_back(l);
        }

        void pop() {
	  if (stack.size() > 1)
            stack.pop_back();
          assert(stack.back());
        }


	void setLocation (MiniMC::Model::Location* l) {
	  stack.back () = l;
	}
	
	
        auto& cur() const {
          assert(stack.size());
          return stack.back();
        }
        virtual MiniMC::Hash::hash_t hash() const {
	  MiniMC::Hash::Hasher hash;
	  for (auto& t : stack)
	    hash << *t;
	  return hash;
	}
	
        std::vector<MiniMC::Model::Location*> stack;
      };
    } // namespace Location
  }   // namespace CPA
} // namespace MiniMC

namespace MiniMC {
  namespace CPA {
    namespace Location {
      class State : public MiniMC::CPA::CFAState,
		    private MiniMC::CPA::LocationInfo
      {
      public:
        State(const std::vector<LocationState>& locations) : locations(locations) {
        }

        State(const State&) = default;
	
	
        virtual MiniMC::Hash::hash_t hash() const override {
	  MiniMC::Hash::Hasher hash;
	  for (auto& t : locations)
            hash << t;
	  return hash;
	}

	virtual std::shared_ptr<MiniMC::CPA::Location::State> lcopy() const { return std::make_shared<State>(*this); }
        virtual std::shared_ptr<MiniMC::CPA::CFAState> copy() const override { return lcopy(); }
	
        size_t nbOfProcesses() const override { return locations.size(); }
        MiniMC::Model::Location& getLocation(size_t i) const override { return *locations.at(i).cur(); }
        void setLocation(size_t i, MiniMC::Model::Location* l) { 
	  locations[i].setLocation(l);
        }
        void pushLocation(size_t i, MiniMC::Model::Location* l) { locations[i].push(l); }
        void popLocation(size_t i) { locations[i].pop(); }
        
	virtual const MiniMC::CPA::LocationInfo& getLocationState () const {return *this;}
      
	
      private:
        std::vector<LocationState> locations;
      };

      MiniMC::CPA::State_ptr<CFAState> MiniMC::CPA::Location::Transferer::doTransfer(const CFAState& s, const Transition& transition ) {
	const MiniMC::Model::Edge& edge = *transition.edge;
	proc_id id = transition.proc;
        auto& state = static_cast<const State&>(s);
        assert(id < state.nbOfProcesses());
	
	if (edge.getFrom ().get () != &state.getLocation(id))
	  return nullptr;
	
	auto nstate = state.lcopy();
	nstate->setLocation(id, edge.getTo().get());
	if (!edge.getInstructions ()) {
	  return nstate;
	}

	auto& inst = edge.getInstructions ().last();
	if (inst.getOpcode() == MiniMC::Model::InstructionCode::Call) {
	  auto& content = inst.getOps<MiniMC::Model::InstructionCode::Call> ();
	  if (content.function->isConstant()) {
	    auto func = MiniMC::Model::visitValue (
						   MiniMC::Model::Overload {
						     [this](const MiniMC::Model::Pointer& t) -> MiniMC::Model::Function_ptr {
						       auto loadPtr = t.getValue ();
						       auto func = prgm.getFunction(loadPtr.base);
						       return func;
						     },
						       [this] (const MiniMC::Model::Pointer32& t) -> MiniMC::Model::Function_ptr {
							 auto loadPtr = t.getValue ();
							 auto func = prgm.getFunction(loadPtr.base);
							 return func;
						       },
						       [this] (const MiniMC::Model::SymbolicConstant& t) -> MiniMC::Model::Function_ptr {
							 auto symb = t.getValue ();
							 auto func = prgm.getFunction(symb);
							 return func;
						       },
						       [](const auto&) -> MiniMC::Model::Function_ptr {							 
							 throw MiniMC::Support::Exception("Shouldn't happen");
						       }
						       }
						   ,*content.function);
	    
	    nstate->pushLocation(id, func->getCFA().getInitialLocation().get());
	    return nstate;
	  }
	}
	
	else if (MiniMC::Model::isOneOf<MiniMC::Model::InstructionCode::RetVoid,
		 MiniMC::Model::InstructionCode::Ret>(inst)) {
	  nstate->popLocation(id);
	  return nstate;
	}
	
	return nstate;
	
      }
      
    

      State_ptr<CFAState> MiniMC::CPA::Location::CPA::makeInitialState(const InitialiseDescr& p) {
        std::vector<LocationState> locs;
        for (auto& f : p.getEntries()) {
          locs.emplace_back();
          locs.back().push(f->getCFA().getInitialLocation().get());
        }
        return std::make_shared<State>(locs);
      }

      
    } // namespace Location
  }   // namespace CPA
} // namespace MiniMC
