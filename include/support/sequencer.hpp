#ifndef _SEQUENCER__
#define _SEQUENCER__

#include <memory>
#include <vector>

namespace MiniMC {
  namespace Support {
    template <class T>
    struct Sink {
      virtual bool run(T&) { return true; };
    };

    template <class T>
    class Sequencer {
    public:
      template <class P, class... Args>
      Sequencer<T>& add(Args... args) {
        sinks.push_back(std::make_unique<P>(args...));
        return *this;
      }

      bool run(T& t) {
        for (auto& s : sinks) {
          if (!s->run(t)) {
            return false;
          }
        }
        return true;
      }

    private:
      std::vector<std::unique_ptr<Sink<T>>> sinks;
    };

    template <class T, class W, class... Args>
    struct SequenceWrapper : public Sink<T> {
      SequenceWrapper(Args... args) : wrapped(args...) {}
      virtual bool run(T& t) override {
        wrapped.run(t);
        return true;
      };

    private:
      W wrapped;
    };

  } // namespace Support
} // namespace MiniMC

#endif
