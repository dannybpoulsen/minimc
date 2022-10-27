#ifndef _LOOPANALYSIS__
#define _LOOPANALYSIS__

#include "model/cfg.hpp"
#include <memory>
#include <set>

namespace MiniMC {
  namespace Model {
    namespace Analysis {
      class Loop : public std::enable_shared_from_this<Loop> {
      public:
        Loop(const MiniMC::Model::Location_ptr& header) : header(header) {}

        auto getHeader() const { return header; }

        auto latch_begin() const { return latches.begin(); }
        auto latch_end() const { return latches.end(); }

        auto body_begin() const { return body.begin(); }
        auto body_end() const { return body.end(); }

        auto exiting_begin() const { return exiting.begin(); }
        auto exiting_end() const { return exiting.end(); }

        auto internal_begin() const { return internal.begin(); }
        auto internal_end() const { return internal.end(); }
        auto back_begin() const { return back_edges.begin(); }
        auto back_end() const { return back_edges.end(); }

        bool contains(const MiniMC::Model::Location_ptr& c) {
          return body.count(c) || c == header;
        }

        void insertLatch(const MiniMC::Model::Location_ptr& latch) {
          latches.insert(latch);
        }

        void insertBody(const MiniMC::Model::Location_ptr& latch) {
          body.insert(latch);
        }

        auto body_insert() {
          return std::inserter(body, body.begin());
        }

        void finalise() {
          exiting.clear();
          internal.clear();
          for (auto& b : body) {
            std::for_each(b->ebegin(), b->eend(), [&](const auto& e) { this->insertEdge(e); });
          }

          /*for (auto& b : latches) {
			std::for_each (b->ebegin (),b->eend(),[&](const auto& e) {this->insertEdge(e);});
			}*/

          std::for_each(header->ebegin(), header->eend(), [&](const auto& e) { this->insertEdge(e); });
        }

        auto size() const { return latches.size() + body.size() + 1; }

        void addChildLoop(std::unique_ptr<Loop>& l) {
          child.push_back(std::move(l));
        }

        void setParent(Loop* p) {
          parent = p;
        }

        Loop* getParent() {
          return parent;
        }

        template <class Iterator>
        void enumerate_loops(Iterator iter) {
          for (auto& l : child) {
            l->enumerate_loops(iter);
          }
          iter = this;
        }

      private:
        void insertEdge(MiniMC::Model::Edge* e) {
          if (e->getTo() == header) {
            back_edges.insert(e);
          } else if (contains(e->getTo())) {
            internal.insert(e);
          } else {
            exiting.insert(e);
          }
        }

        MiniMC::Model::Location_ptr header;
        std::set<MiniMC::Model::Location_ptr> latches;
        std::set<MiniMC::Model::Location_ptr> body;
        std::set<MiniMC::Model::Edge*> exiting;
        std::set<MiniMC::Model::Edge*> internal;
        std::set<MiniMC::Model::Edge*> back_edges;

        std::vector<std::unique_ptr<Loop>> child;
        Loop* parent = nullptr;
      };

      using Loop_ptr = std::unique_ptr<Loop>;

      class LoopInfo {
      public:
        auto begin() const { return loops.begin(); }
        auto end() const { return loops.end(); }
        template <class Iterator>
        auto deleteLoop(Iterator it) { return loops.erase(it); }
        void addLoop(Loop_ptr& loop) {
          loops.push_back(std::move(loop));
        }

        void nestLoops() {
          std::sort(loops.begin(), loops.end(), [](auto& l, auto& r) { return l->size() < r->size(); });
          auto rit = loops.begin();
          auto rend = loops.end();
          for (; rit != rend; ++rit) {

            for (auto ritpar = rit + 1; ritpar != loops.end(); ++ritpar) {
              if ((*ritpar)->contains((*rit)->getHeader())) {
                (*rit)->setParent(ritpar->get());
                (*ritpar)->addChildLoop(*rit);
                rit = loops.erase(rit);
                break;
              }
            }
          }
        }

        template <class Iterator>
        void enumerate_loops(Iterator iter) {
          for (auto& l : loops) {
            l->enumerate_loops(iter);
          }
        }

      private:
        std::vector<Loop_ptr> loops;
      };

      LoopInfo createLoopInfo(const MiniMC::Model::CFA& ptr);

      class LoopAnalyser {
      public:
        LoopAnalyser(const MiniMC::Model::Program_ptr&) {
        }

        LoopInfo getLoopsFunction(const MiniMC::Model::Function_ptr& func) {
          return createLoopInfo(func->getCFA());
        }
      };

    } // namespace Analysis
  }   // namespace Model
} // namespace MiniMC

#endif
