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
    
  }
}

#endif _SEQUENCER__
