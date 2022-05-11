/**
 * @file   interface.hpp
 * @date   Mon Apr 20 17:18:43 2020
 *
 * MiniMC uses something called CPAs to encapsulate different
 * "verification views" on  a program. For instance one CPA may
 * "track" the program locations of all processes/threads while
 * another tracks their variable values and memory. Combining these
 * two together provides an explicit verification context
 *
 *
 */
#ifndef _CPA_INTERFACE__
#define _CPA_INTERFACE__

#include "cpa/state.hpp"
#include "hash/hashing.hpp"
#include "model/cfg.hpp"
#include "support/feedback.hpp"
#include "support/sequencer.hpp"
#include <memory>
#include <ostream>
#include <unordered_map>

namespace MiniMC {
  namespace CPA {

    struct InitialiseDescr {
    public:
      InitialiseDescr(std::vector<MiniMC::Model::Function_ptr> entries,
                      MiniMC::Model::HeapLayout heap,
                      MiniMC::Model::InstructionStream init,
                      const MiniMC::Model::Program& program) : entries(std::move(entries)),
                                                               heap(std::move(heap)),
                                                               init(std::move(init)),
                                                               prgm(program) {}

      auto& getEntries() const { return entries; }
      auto& getHeap() const { return heap; }
      auto& getInit() const { return init; }
      auto& getProgram() const { return prgm; }

    private:
      std::vector<MiniMC::Model::Function_ptr> entries;
      MiniMC::Model::HeapLayout heap;
      MiniMC::Model::InstructionStream init;
      const MiniMC::Model::Program& prgm;
    };

    struct StateQuery {
      virtual State_ptr makeInitialState(const InitialiseDescr&) = 0;
    };

    using StateQuery_ptr = std::shared_ptr<StateQuery>;

    /**
     * The Tranferer generates successor for States
     */
    struct Transferer {
      /**
       * Comput the successor state of \p s by performing edge \p e for
       * process \p
       *
       * @param s
       *
       * @return the resulting State of nullptr if the edge cannot be
       * performed (which may happen for instance) when a guard is
       * false)
       */
      virtual State_ptr doTransfer(const State_ptr&, const MiniMC::Model::Edge_ptr&, proc_id) = 0;
    };

    using Transferer_ptr = std::shared_ptr<Transferer>;

    struct Joiner {
      /**
       * Join two  states \p l and \p r with each other
       *
       * @return  the joined state of nullptr if the states cannot be
       * merged.
       */
      virtual State_ptr doJoin(const State_ptr&, const State_ptr&) = 0;

      /**
       * Test if \p l covers \p r i.e. whether the behaviour of \l
       * includes that of \p r
       */
      virtual bool covers(const State_ptr&, const State_ptr&) = 0;
    };

    using Joiner_ptr = std::shared_ptr<Joiner>;

    struct ICPA {
      virtual ~ICPA() {}
      virtual StateQuery_ptr makeQuery() const = 0;
      virtual Transferer_ptr makeTransfer(const MiniMC::Model::Program& ) const = 0;
      virtual Joiner_ptr makeJoin() const = 0;
    };

    using CPA_ptr = std::shared_ptr<ICPA>;

    template <
        class Query,
        class Transfer,
        class Joiner>
    struct CPADef : public ICPA {
      virtual StateQuery_ptr makeQuery() const { return std::make_shared<Query>(); }
      virtual Transferer_ptr makeTransfer(const MiniMC::Model::Program& prgm) const { return std::make_shared<Transfer>(prgm); }
      virtual Joiner_ptr makeJoin() const { return std::make_shared<Joiner>(); }
    };
    
  } // namespace CPA
} // namespace MiniMC

#endif
