
#ifndef MINIMC_VERDICT_HPP
#define MINIMC_VERDICT_HPP

#include "algorithms/algorithms.hpp"
#include "cpa/interface.hpp"
#include <variant>


namespace MiniMC {
namespace Algorithms {
namespace Reachability {
namespace Verdict {

enum class Verdict { ReachabilityFound, ReachabilityNotFound };

template<Verdict V>
struct VerdictData{
  static const bool isFound = false;
};

struct FoundContent {
  MiniMC::CPA::AnalysisState state;
};

template<>
struct VerdictData<Verdict::ReachabilityFound> {
  static const bool isFound = true;
  using Content = FoundContent;
};

template<>
struct VerdictData<Verdict::ReachabilityNotFound>{
  static const bool isFound = false;
  using Content = int;
};

using Verdict_content = std::variant<FoundContent,int>;

struct Details {
public:
  Details(Verdict verdict, Verdict_content verdictcontent) : verdict(verdict), content(verdictcontent){};
  Verdict getVerdict(){return verdict;}

  template<Verdict V>
  auto& getContent(){
    return std::get<typename VerdictData<V>::Content> (content);
  }

private:
  Verdict verdict;
  Verdict_content content;
};

template<Verdict V>
Details createResult(const typename VerdictData<V>::Content content){
  return Details(V,content);
}



} // namespace Verdict
} // namespace Reachability
} // namespace Algorithms
} // namespace MiniMC



#endif // MINIMC_VERDICT_HPP
