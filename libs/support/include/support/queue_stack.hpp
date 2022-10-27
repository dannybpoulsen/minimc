#ifndef _QUEUE_STACK__
#define _QUEUE_STACK__

#include <cassert>
#include <deque>
#include <memory>
#include <vector>

namespace MiniMC {
  namespace Support {

    template <class T>
    class Stack {
    public:
      using T_ptr = std::shared_ptr<T>;
      bool empty() const {
        return thestack.empty();
      }

      void insert(const T_ptr& state) {
        thestack.push_back(state);
      }

      T_ptr pull() {
        assert(!empty());
        auto res = thestack.back();
        thestack.pop_back();
        return res;
      }

      auto begin() { return thestack.begin(); }
      auto end() { return thestack.begin(); }

      auto size() const { return thestack.size(); }

    private:
      std::vector<T_ptr> thestack;
    };

    template <class T>
    class Queue {

    public:
      using T_ptr = std::shared_ptr<T>;
      bool empty() const {
        return !thequeue.size();
      }

      void insert(const T_ptr& state) {
        thequeue.push_front(state);
      }

      T_ptr pull() {
        assert(!empty());
        auto res = thequeue.front();
        thequeue.pop_front();
        return res;
      }

      auto begin() { return thequeue.begin(); }
      auto end() { return thequeue.begin(); }

      auto size() const { return thequeue.size(); }

    private:
      std::deque<T_ptr> thequeue;
    };

  } // namespace Support
} // namespace MiniMC

#endif
