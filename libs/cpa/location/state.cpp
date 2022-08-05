#include "cpa/interface.hpp"
#include "cpa/location.hpp"
#include "hash/hashing.hpp"
#include "model/cfg.hpp"
#include "support/pointer.hpp"


namespace MiniMC {
  namespace CPA {
    namespace Location {

      struct LocationState {
        void push(gsl::not_null<MiniMC::Model::Location*> l) {
	  stack.push_back(l.get());
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

        virtual std::ostream& output(std::ostream& os) const override {
          os << "[ ";
          for (auto l : locations) {
            assert(l.cur());
            os << l.cur()->getInfo() << ", ";
          }
          return os << "]";
        }
        virtual MiniMC::Hash::hash_t hash() const override {
	  MiniMC::Hash::Hasher hash;
	  for (auto& t : locations)
            hash << t;
	  return hash;
	}

	virtual std::shared_ptr<MiniMC::CPA::Location::State> lcopy() const { return std::make_shared<State>(*this); }
        virtual std::shared_ptr<MiniMC::CPA::CommonState> copy() const override { return lcopy(); }

        size_t nbOfProcesses() const override { return locations.size(); }
        MiniMC::Model::Location_ptr getLocation(size_t i) const override { return locations.at(i).cur()->shared_from_this(); }
        void setLocation(size_t i, MiniMC::Model::Location* l) { 
	  locations[i].setLocation(l);
        }
        void pushLocation(size_t i, MiniMC::Model::Location* l) { locations[i].push(l); }
        void popLocation(size_t i) { locations[i].pop(); }
        
        virtual bool assertViolated() const override {
          for (auto& locState : locations) {
            if (locState.cur()->getInfo().template is<MiniMC::Model::Attributes::AssertViolated>())
              return true;
          }
          return false;
        }

        virtual bool hasLocationAttribute(MiniMC::Model::AttrType tt) const override  {
          for (auto& locState : locations) {
            if (locState.cur()->getInfo().isFlagSet(tt))
              return true;
          }
          return false;
        }

	virtual const MiniMC::CPA::LocationInfo& getLocationState () const {return *this;}
      
	
      private:
        std::vector<LocationState> locations;
        std::vector<bool> ready;
      };

      MiniMC::CPA::CommonState_ptr MiniMC::CPA::Location::Transferer::doTransfer(const CommonState_ptr& s, const MiniMC::Model::Edge* edge, proc_id id) {
        auto state = static_cast<const State*>(s.get());
        assert(id < state->nbOfProcesses());
        if (edge->getFrom() == state->getLocation(id)) {
          auto nstate = state->lcopy();
          nstate->setLocation(id, edge->getTo().get());

          if (edge->getInstructions () ) {
            auto& inst = edge->getInstructions ().last();
            if (inst.getOpcode() == MiniMC::Model::InstructionCode::Call) {
	      auto& content = inst.getOps<MiniMC::Model::InstructionCode::Call> ();
	      if (content.function->isConstant()) {
                auto constant = std::static_pointer_cast<MiniMC::Model::Pointer>(content.function);
                pointer_t loadPtr = constant->getValue ();
                auto func = prgm.getFunction(MiniMC::Support::getFunctionId(loadPtr));
                nstate->pushLocation(id, func->getCFA().getInitialLocation().get());
              } else
                return nullptr;
            }

            else if (MiniMC::Model::isOneOf<MiniMC::Model::InstructionCode::RetVoid,
                                            MiniMC::Model::InstructionCode::Ret>(inst)) {
              nstate->popLocation(id);
            }
          }
          return nstate;
        }

        return nullptr;
      }

      CommonState_ptr MiniMC::CPA::Location::StateQuery::makeInitialState(const InitialiseDescr& p) {
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
