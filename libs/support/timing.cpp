
#include <cassert>
#include <chrono>
#include <string>
#include  <unordered_map>
#include "support/timing.hpp"

namespace MiniMC {
  namespace Support {
	using clock_t = std::chrono::steady_clock;
	using time_t = clock_t::time_point;
	using duration_t = clock_t::duration;
	
	
	
	struct Timer::inner_t {
	  //elapsed_t total = {.seconds = 0, .milliseconds = 0};
	  duration_t duration;
	  time_t time_started = clock_t::now (); 
	  bool started = false;
	  std::string name;
	};

	
	Timer::Timer (const std::string& name,bool start) {
	  _inner = std::make_unique<inner_t> ();
	  _inner->name = name;
	  if (start)
		startTimer ();
	}
	
	Timer::~Timer () {}

	void Timer::startTimer () {
	  assert(!_inner->started);
	  _inner->started = true;
	  _inner->time_started = clock_t::now ();

	}

	
	
	void Timer::stopTimer () {
	  assert(_inner->started);
	  auto duration = _inner->duration+ (clock_t::now () - _inner->time_started);
	  _inner->started = false;
	  
	}

	
	elapsed_t Timer::current () const {
	  assert(_inner->started);
	  return elapsed_t {.milliseconds = std::chrono::duration_cast<std::chrono::milliseconds> (clock_t::now() - _inner->time_started).count()};
	  
	}
	
	elapsed_t Timer::total () const  {
	  return elapsed_t {.milliseconds = std::chrono::duration_cast<std::chrono::milliseconds> (_inner->duration).count()};
	}

	Timer& Timer::getTimer (const std::string& name) {
	  static std::unordered_map<std::string, std::unique_ptr<Timer>> timers;
	  if (!timers.count(name)) {
		timers.insert(std::make_pair (name,std::make_unique<Timer> (name)));
	  }
	  return *timers.at (name);
	}
	
	
  }
}
