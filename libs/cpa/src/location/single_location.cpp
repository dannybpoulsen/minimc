#include "cpa/interface.hpp"
#include "cpa/location.hpp"
#include "hash/hashing.hpp"
#include "model/cfg.hpp"

namespace MiniMC {
  namespace CPA {
    namespace SingleLocation {
      class State : public MiniMC::CPA::CFAState,
		    private MiniMC::CPA::LocationInfo
      {
      public:
        State(const MiniMC::Model::Location_ptr& loc) : location(loc.get()) {
        }

        State(const State&) = default;

        virtual std::ostream& output(std::ostream& os) const override {
          return os << "[" << location->getInfo() << "]";
        }
        virtual MiniMC::Hash::hash_t hash() const override {
          MiniMC::Hash::Hasher  hash;
	  hash << *location;
          return hash;;
        }
        virtual std::shared_ptr<MiniMC::CPA::SingleLocation::State> lcopy() const { return std::make_shared<State>(*this); }
        virtual std::shared_ptr<MiniMC::CPA::CommonState> copy() const override { return lcopy(); }

        size_t nbOfProcesses() const override { return 1; }
        MiniMC::Model::Location_ptr getLocation(proc_id ) const override {
          return location->shared_from_this();
        }
        void setLocation(MiniMC::Model::Location* l) {
          location = l;
        }
        
        
        virtual bool hasLocationAttribute(MiniMC::Model::AttrType tt) const override {
          return location->getInfo().isFlagSet(tt);
        }

        std::shared_ptr<State> join(const State& oth) const {
          if (location == oth.location) {
            auto nstate = std::make_shared<State>(location->shared_from_this());
            return nstate;
          }
          return nullptr;
        }

	const LocationInfo& getLocationState () const override{return *this;}
	
      private:
        MiniMC::Model::Location* location;
        bool ready;
      };

      MiniMC::CPA::CommonState_ptr Transferer::doTransfer(const CommonState_ptr& s, const MiniMC::Model::Edge_ptr& edge, proc_id) {
        auto state = static_cast<const MiniMC::CPA::SingleLocation::State*>(s.get());
        
        if (edge->getFrom() == state->getLocation(0)) {
          auto nstate = state->lcopy();
          nstate->setLocation(edge->getTo().get());

          if (edge->getInstructions()) {
            auto& inst = edge->getInstructions().last();
            if (inst.getOpcode() == MiniMC::Model::InstructionCode::Call) {
              //throw NotImplemented<MiniMC::Model::InstructionCode::Call> ();
	      
            }

            else if (MiniMC::Model::isOneOf<MiniMC::Model::InstructionCode::RetVoid,
                                            MiniMC::Model::InstructionCode::Ret>(inst)) {
              //throw NotImplemented<MiniMC::Model::InstructionCode::RetVoid> ();
            }
          }
          return nstate;
        }

        return nullptr;
      }

      CommonState_ptr StateQuery::makeInitialState(const InitialiseDescr& p) {
        std::vector<MiniMC::Model::Location_ptr> locs;
        for (auto& f : p.getEntries()) {
          locs.push_back(f->getCFA().getInitialLocation());
        }
        assert(locs.size() == 1);
        return std::make_shared<State>(locs[0]);
      }

      CommonState_ptr Joiner::doJoin(const CommonState_ptr& l, const CommonState_ptr& r) {
        auto lstate = std::static_pointer_cast<const State>(l);
        auto rstate = std::static_pointer_cast<const State>(r);
        return lstate->join(*rstate);
      }

    } // namespace SingleLocation
  }   // namespace CPA
} // namespace MiniMC
