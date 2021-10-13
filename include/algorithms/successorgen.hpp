#ifndef _SUCCESSOR_GEN__
#define _SUCCESSOR_GEN__

#include "cpa/interface.hpp"
#include "support/exceptions.hpp"
#include "support/types.hpp"
#include <gsl/pointers>

namespace MiniMC {
  namespace Algorithms {
    enum class ErrorFlags {
      AssertViolated = 1,
    };

    struct Successor {
      MiniMC::CPA::State_ptr state;
      MiniMC::Model::Edge_ptr edge;
      MiniMC::CPA::proc_id proc;
      MiniMC::uint8_t eflags;
      bool hasErrors() const { return eflags; }
    };

    /** 
     * Generator of successor states. 
     * This class takes a State as input, and exposes an iterator interface to generate all possible successors.
     * \tparam StateQuery class used for Querying the state for location and successors
     * \tparam Transfer Transfer relation for executing an Edge in a state
     */
    class Generator {
      class Iterator {
      public:
        static Iterator makeBegin(const MiniMC::CPA::State_ptr& pt,
                                  MiniMC::CPA::Transferer_ptr transfer) {
          auto loc = pt->getLocation(0);
          return Iterator(pt, 0, pt->nbOfProcesses(), loc->ebegin(), loc->eend(), transfer);
        }

        static Iterator makeEnd(const MiniMC::CPA::State_ptr& pt,
                                MiniMC::CPA::Transferer_ptr transfer) {
          auto proc = pt->nbOfProcesses();
          auto loc = pt->getLocation(proc - 1);
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
        Iterator(const MiniMC::CPA::State_ptr& st, MiniMC::CPA::proc_id proc, MiniMC::CPA::proc_id lproc, MiniMC::Model::Location::edge_iterator beg, MiniMC::Model::Location::edge_iterator end,
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
              auto loc = curState->getLocation(proc); //
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
                succ.eflags = static_cast<MiniMC::uint8_t>(ErrorFlags::AssertViolated);
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

        MiniMC::CPA::State_ptr curState;
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

      auto begin_it(const MiniMC::CPA::State_ptr& state) {
        if (state->nbOfProcesses() == 0) {
          throw MiniMC::Support::Exception("No Processes to generate sucessors for");
        }
	
        return Iterator::makeBegin(state, transfer);
      }

      auto end_it(const MiniMC::CPA::State_ptr& state) { return Iterator::makeEnd(state, transfer); }

      auto generate(const MiniMC::CPA::State_ptr& state) {
        return std::make_pair(begin_it(state), end_it(state));
      }

    private:
      MiniMC::CPA::Transferer_ptr transfer;
    };

  } // namespace Algorithms
} // namespace MiniMC

#endif
