
#ifndef MINIMC_VERDICT_HPP
#define MINIMC_VERDICT_HPP

#include "algorithms/algorithms.hpp"
#include "cpa/interface.hpp"
#include <variant>


namespace MiniMC {
namespace Algorithms {
namespace Reachability {
namespace Verdict {

enum class Verdict { Found, NotFound };

template<Verdict V>
struct VerdictData{
  static const bool isFound = false;
};

struct FoundContent {
  MiniMC::CPA::AnalysisState& state;
};

template<>
struct VerdictData<Verdict::Found> {
  static const bool isFound = true;
  using Content = FoundContent;
};

template<>
struct VerdictData<Verdict::NotFound>{
  static const bool isFound = false;
  using Content = int;
};

using Verdict_content = std::variant<FoundContent,int>;

struct Result {
public:
  Result(Verdict verdict, Verdict_content verdictcontent) : verdict(verdict), content(verdictcontent){};
  Verdict getVerdict(){return verdict;}
  Verdict_content getContent(){return content;}

private:
  Verdict verdict;
  Verdict_content content;
};

template<Verdict V>
Result createResult(const typename VerdictData<V>::Content content){
  return Result(V,content);
}



} // namespace Verdict
} // namespace Reachability
} // namespace Algorithms
} // namespace MiniMC



#endif // MINIMC_VERDICT_HPP
