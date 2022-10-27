#ifndef MINIMC_STATEMAP_HPP
#define MINIMC_STATEMAP_HPP

#include "cpa/state.hpp"
namespace MiniMC {
namespace Interpreter {

class StateMap {
public:
  StateMap(MiniMC::CPA::AnalysisState current) { map["current"] = current; }

  MiniMC::CPA::AnalysisState get(std::string key) { return map[key]; }

  MiniMC::CPA::AnalysisState operator[](std::string key) { return map[key]; }

  void set(std::string rhand, std::string lhand) { map[rhand] = map[lhand]; };
  void set(std::string rhand, MiniMC::CPA::AnalysisState lhand) {
    map[rhand] = lhand;
  };

  bool contains(std::string key) { return map.contains(key); }

private:
  std::unordered_map<std::string, MiniMC::CPA::AnalysisState> map{};
};

} // namespace Interpreter
} // namespace MiniMC

#endif // MINIMC_STATEMAP_HPP
