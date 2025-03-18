#ifndef __RANDOM__
#define __RANDOM__

#include <random>

namespace MiniMC {
  namespace Support {
    class Random {
    private:
      auto& getEngine(){
	static thread_local std::default_random_engine engine(std::random_device{}());
	return engine;
      }
      
    public:
      template<class T>
      T uniform_int (T a, T b) {
	return std::uniform_int_distribution<T> {a,b} (getEngine()); 
      }
    
    };

    
  }
}


#endif
