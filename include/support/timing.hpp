#ifndef _TIMING__
#define _TIMING__

#include <memory>

namespace MiniMC {
  namespace Support {

	struct elapsed_t {
	  long milliseconds;
	};
	
	class Timer {
	public:
	  Timer (const std::string& name,bool start= false);
	  ~Timer ();
	  void startTimer ();
	  void stopTimer ();
	  elapsed_t current () const;
	  elapsed_t total () const ;
	  static Timer& getTimer (const std::string& name);
	private:
	  
	  struct inner_t;
	  std::unique_ptr<inner_t> _inner;
	  
	};

	
	
	class AutoTime {
	public:
	  AutoTime (Timer& t) : timer(t) {
		timer.startTimer ();
	  }

	  ~AutoTime () {
		timer.stopTimer ();
	  }
	  
	private:
	  Timer& timer;
	};


	Timer& getTimer (const std::string&); 
	
  }
}


#endif
