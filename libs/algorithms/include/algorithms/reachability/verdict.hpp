
#ifndef MINIMC_VERDICT_HPP
#define MINIMC_VERDICT_HPP

#include <variant>

#include "algorithms/algorithms.hpp"
#include "cpa/interface.hpp"
#include "storage/storage.hpp"


namespace MiniMC {
namespace Algorithms {
namespace Reachability {
namespace Verdict {

enum class Verdict { ReachabilityFound, ReachabilityNotFound };

template<Verdict V>
struct VerdictData{
};

struct ReachabilityFoundContent {
  MiniMC::CPA::AnalysisState state;
  MiniMC::Storage::HashStorage storage;
};

template<>
struct VerdictData<Verdict::ReachabilityFound> {
  using Content = ReachabilityFoundContent;
};

template<>
struct VerdictData<Verdict::ReachabilityNotFound>{
  using Content = int;
};

using Verdict_content = std::variant<ReachabilityFoundContent,int>;

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
