#include <unordered_map>
#include "algorithms/algorithm.hpp"

#include "plugin.hpp"

namespace {
  auto& getMap () {
	static std::unordered_map<std::string,CommandRegistrar*> map;
	return map;
  }
}

void registerCommand (const std::string& s,CommandRegistrar& r) {
  getMap().insert(std::make_pair(s,&r));
}

bool isCommand (const std::string& s) {
  return getMap().count (s);
}

subcommandfunc getCommand (const std::string& s) {
  return getMap().at(s)->getFunction ();
}



