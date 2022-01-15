/**
 * @file   psuccessorgen.hpp
 * @date   Mon Apr 20 17:00:06 2020
 * 
 * @brief  
 * 
 * 
 */
#ifndef _PSUCCESSOR_GEN__
#define _PSUCCESSOR_GEN__

#include "cpa/interface.hpp"
#include "support/exceptions.hpp"
#include "support/random.hpp"
#include "support/types.hpp"
#include <gsl/pointers>

namespace MiniMC {
  namespace Algorithms {
    namespace Proba {
      struct Successor {
        MiniMC::CPA::State_ptr state;
        MiniMC::Model::Edge_ptr edge;
        MiniMC::CPA::proc_id proc;
      };

      /**
       * Works similar to \ref MiniMC::Algorithms::Generator with the execption, that it only generates one State and that state is randomly selected when the Generator is created.
       */
      class Generator {
        class Iterator {
        public:
          Successor& operator*() { return succ; }
          Successor* operator->() { return &succ; }

          Iterator& operator++() {
            update();
            return *this;
          }

          bool operator==(const Iterator&) const {
            return false;
          }

          bool operator!=(const Iterator&) const {
            return true;
          }

        public:
          Iterator(const MiniMC::CPA::State_ptr& st, MiniMC::CPA::Transferer_ptr transfer) : curState(st),
                                                                                             transfer(transfer) {
            update();
          }

          void update() {
            auto proc = curState->nbOfProcesses();
            std::vector<MiniMC::proc_t> procs;
            auto insert = std::back_inserter(procs);
            for (size_t i = 0; i < proc; ++i) {
              if (curState->getLocation(i)->hasOutgoingEdge())
                insert = i;
            }
            if (procs.size()) {
              MiniMC::Support::RandomNumber random;
              succ.proc = random.selectUniform<MiniMC::proc_t>(procs);
              auto it = curState->getLocation(succ.proc)->ebegin();
              auto end = curState->getLocation(succ.proc)->eend();
              for (; it != end; ++it) {
                auto edge = *it;
                succ.state = transfer->doTransfer(curState, edge, succ.proc);
                if (succ.state) {
                  succ.edge = edge;
                  return;
                }
              }
            }
            succ.edge = nullptr;
            succ.state = nullptr;
          }

          MiniMC::CPA::State_ptr curState;
          Successor succ;
          MiniMC::CPA::Transferer_ptr transfer;
        };

      public:
        using iterator = Iterator;
        Generator(MiniMC::CPA::Transferer_ptr t) : transfer(t) {
        }

        auto begin_it(const MiniMC::CPA::State_ptr& state) {
          return Iterator(state, transfer);
        }

        auto end_it(const MiniMC::CPA::State_ptr& state) { return Iterator(state, transfer); }

        auto generate(const MiniMC::CPA::State_ptr& state) {
          return std::make_pair(begin_it(state), end_it(state));
        }

      private:
        MiniMC::CPA::Transferer_ptr transfer;
      };

    } // namespace Proba
  }   // namespace Algorithms
} // namespace MiniMC
#endif
