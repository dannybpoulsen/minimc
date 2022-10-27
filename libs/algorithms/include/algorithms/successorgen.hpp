#ifndef _SUCCESSOR_GEN__
#define _SUCCESSOR_GEN__

#include "cpa/interface.hpp"
#include "support/exceptions.hpp"
#include "host/types.hpp"

namespace MiniMC {
  namespace Algorithms {
    /*    enum class ErrorFlags {
      AssertViolated = 1,
    };

    struct Successor {
      MiniMC::CPA::CommonState_ptr state;
      MiniMC::Model::Edge_ptr edge;
      MiniMC::CPA::proc_id proc;
      MiniMC::BV8 eflags;
      bool hasErrors() const { return eflags; }
    };

    class Generator {
      class Iterator {
      public:
        static Iterator makeBegin(const MiniMC::CPA::CommonState_ptr& pt,
                                  MiniMC::CPA::Transferer_ptr transfer) {
          auto loc = pt->getLocationState().getLocation(0);
          return Iterator(pt, 0, pt->getLocationState().nbOfProcesses(), loc->ebegin(), loc->eend(), transfer);
        }

        static Iterator makeEnd(const MiniMC::CPA::CommonState_ptr& pt,
                                MiniMC::CPA::Transferer_ptr transfer) {
          auto proc = pt->getLocationState().nbOfProcesses();
          auto loc = pt->getLocationState().getLocation(proc - 1);
          return Iterator(pt, proc - 1, proc, loc->eend(), loc->eend(), transfer);
        }

        Successor& operator*() { return succ; }
        Successor* operator->() { return &succ; }

        Iterator& operator++() {
          ++iter;
          update();
          return *this;
        }

        bool operator==(const Iterator& oth) const {
          return oth.proc == proc &&
                 oth.iter == iter &&
                 oth.end == end;
        }

        bool operator!=(const Iterator& oth) const {
          return !(*this == oth);
        }

      private:
        Iterator(const MiniMC::CPA::CommonState_ptr& st, MiniMC::CPA::proc_id proc, MiniMC::CPA::proc_id lproc, MiniMC::Model::Location::edge_iterator beg, MiniMC::Model::Location::edge_iterator end,
                 const MiniMC::CPA::Transferer_ptr& transfer) : curState(st),
                                                                proc(proc),
                                                                last_proc(lproc),
                                                                iter(beg),
                                                                end(end),
                                                                transfer(transfer) {
          update();
        }

        void update() {
          bool done = false;
          while (!done) {
            while (iter == end && proc < last_proc) {
              proc++;
              if (proc >= last_proc)
                break;
              assert(proc <= last_proc);
              auto loc = curState->getLocationState ().getLocation(proc); //
              iter = loc->ebegin();
              end = loc->eend();
            }

            if (proc < last_proc) {
              succ.proc = proc;
              succ.edge = (*iter);
              succ.eflags = 0;
              try {
                succ.state = transfer->doTransfer(curState, succ.edge, succ.proc);
              } catch (MiniMC::Support::AssertViolated& e) {
                succ.state = nullptr;
                succ.eflags = static_cast<MiniMC::BV8>(ErrorFlags::AssertViolated);
              }

              if (succ.state || succ.eflags) {
                done = true;
              } else {
                ++iter;
              }
            } else {
              done = true;
            }
          }
        }

        MiniMC::CPA::CommonState_ptr curState;
        MiniMC::CPA::proc_id proc;
        MiniMC::CPA::proc_id last_proc;
        MiniMC::Model::Location::edge_iterator iter;
        MiniMC::Model::Location::edge_iterator end;
        Successor succ;
        MiniMC::CPA::Transferer_ptr transfer;
      };

    public:
      using iterator = Iterator;
      Generator(MiniMC::CPA::Transferer_ptr& transfer) : transfer(transfer) {
      }

      auto begin_it(const MiniMC::CPA::CommonState_ptr& state) {
        if (state->getLocationState().nbOfProcesses() == 0) {
          throw MiniMC::Support::Exception("No Processes to generate sucessors for");
        }
	
        return Iterator::makeBegin(state, transfer);
      }

      auto end_it(const MiniMC::CPA::CommonState_ptr& state) { return Iterator::makeEnd(state, transfer); }

      auto generate(const MiniMC::CPA::CommonState_ptr& state) {
        return std::make_pair(begin_it(state), end_it(state));
      }

    private:
      MiniMC::CPA::Transferer_ptr transfer;
    };
    */
    struct EnumResult {
      MiniMC::Model::Edge* edge;
      MiniMC::proc_t proc; 
    };
    
    class EdgeEnumerator {
    public:
      EdgeEnumerator (const MiniMC::CPA::AnalysisState& state) : orig(state),
								 iter(orig.getCFAState ()->getLocationState().getLocation(0)->ebegin ()),
								 end(orig.getCFAState ()->getLocationState().getLocation(0)->eend ())
	
      {
	hasOne = next ();
      }

      bool getNext (EnumResult& res) {
	if (hasOne) {
	  res.edge = *iter;
	  res.proc = proc;
	  hasOne = next ();
	  return true;
	}
	return false;
      }
      
    private:

      bool next () {
	if (init ) {
	  iter = orig.getCFAState ()->getLocationState().getLocation(proc)->ebegin ();
	  end = orig.getCFAState ()->getLocationState().getLocation(proc)->eend ();
	  init = false;
	  return iter!=end;
	}
	
	else {
	  ++iter;
	  if (iter == end) {
	    proc++;
	    if (proc >=orig.getCFAState ()->getLocationState().nbOfProcesses ())
	      return false;
	    iter = orig.getCFAState ()->getLocationState().getLocation(proc)->ebegin ();
	    end = orig.getCFAState ()->getLocationState().getLocation(proc)->eend ();
	    return iter != end;
	  }
	  return true;
	  
	}
	  
      }

      const MiniMC::CPA::AnalysisState& orig;
      proc_t proc{0};
      bool init{true};
      bool hasOne = false;
      MiniMC::Model::Location::edge_iterator iter;
      MiniMC::Model::Location::edge_iterator end;
    };
    
  } // namespace Algorithms
} // namespace MiniMC

#endif
