#ifndef _SEQUENCER__
#define _SEQUENCER__

#include <vector>
#include <memory>

namespace MiniMC {
  namespace Support {
    template<class T>
    struct Sink {
      virtual void run (T& ) {};
    };
    
    template<class T>
    class Sequencer {
    public:
      template<class P,class ...Args >
      Sequencer<T>& add (Args... args) {
	sinks.push_back (std::make_unique<P> (args...));
	return *this;
      }
      
      void run (T& t) {
	for (auto& s : sinks) {
	  s->run (t);
	}
      }
    private:
      std::vector<std::unique_ptr<Sink<T> > > sinks;
    };

    template<class T,class W, class ... Args>
    struct SequenceWrapper : public Sink<T> {
      SequenceWrapper (Args... args) : wrapped (args...) {}
      virtual void run (T& t) override {wrapped.run (t);};
    private:
      W wrapped;
    };
    
  }
}

#endif 
