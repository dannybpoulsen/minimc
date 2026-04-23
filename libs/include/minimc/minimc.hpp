#include "minimc/cpa/interface.hpp"
#include "minimc/support/feedback.hpp"
#include "minimc/loaders/loader.hpp"

namespace MiniMC {
  enum class Result{
    Reachable,
    NotReachable
  };
    
  class ReachabilityChecker {
  public:
    ReachabilityChecker(MiniMC::Support::Interaction inter) : interaction(inter) {
      selectCPA (MiniMC::CPA::CPAType::Concrete);
    }
    bool selectCPA (MiniMC::CPA::CPAType type);
    Result search(const std::string name);
    
    template<typename T>
    void setLoaderOption (const std::string&s, T t) {
      loader.setOption(s,t);
    }
    
  private:
    MiniMC::Support::Interaction interaction;
    MiniMC::CPA::TCPA_ptr cpa;
    MiniMC::Loaders::GenericLoader loader;
  };
}
