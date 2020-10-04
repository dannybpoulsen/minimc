#ifndef _AMANALYSIS_MANAGER__
#define _AMANALYSIS_MANAGER__
#include <memory>
#include "model/analysis/find_location_defs.hpp"

namespace MiniMC {
  namespace Model {
	namespace Analysis {
	enum class AnalysisType {
	  UseDef
	};

	template<AnalysisType>
	struct AnalysisInfo {
	  
	};

	template<>
	struct AnalysisInfo<AnalysisType::UseDef> {
	  using Result = MiniMC::Model::Analysis::ProgramDefs;
	  static void Invalidate (Result& r, MiniMC::Model::Function_ptr& f) {r.forgetFunction (f);}
	};

	
	  
		template<AnalysisType T,AnalysisType... args>
	class TAnalysisManager : public TAnalysisManager<args...> {
	  TAnalysisManager (MiniMC::Model::Program_ptr& prgm) : res (prgm), TAnalysisManager<args...> (prgm) {} 

	  template<AnalysisType res>
	  auto& getAnalysis () {
		if constexpr (T == res) {
		  return res;
		}
		else if constexpr (sizeof...(args) > 0) {
		  return static_cast<TAnalysisManager<args...>&>  (*this).template getAnalysis<res> ();
		}
		
		else {
		  assert(false && "Asked for unknwon unsupported analysis result");
		}
	  }
	private:
	  typename AnalysisInfo<T>::Result res;
	};

	  template<AnalysisType T>
	  class TAnalysisManager<T>  {
	  public:
	  TAnalysisManager (const MiniMC::Model::Program_ptr& prgm) : res (prgm) {} 

	  template<AnalysisType result>
	  auto& getAnalysis () {
		if constexpr (T == result) {
		  return res;
		}
		else {
		  assert(false && "Asked for unknwon unsupported analysis result");
		}
	  }
	  private:
		typename AnalysisInfo<T>::Result res;
	  };
	  
	using Manager = TAnalysisManager<AnalysisType::UseDef>;
	using Manager_ptr = std::shared_ptr<Manager>;
	
  }
}

}
#endif
