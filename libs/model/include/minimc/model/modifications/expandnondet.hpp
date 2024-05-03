#ifndef _MODIFICATIAONS__EXPANDNONDET__
#define _MODIFICATIAONS__EXPANDNONDET__

#include "minimc/model/cfg.hpp"
#include "minimc/model/modifications/modifications.hpp"
#include "minimc/support/feedback.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      class NonDetExpander : public ProgramModifier {
      public:
	NonDetExpander (MiniMC::Model::ConstantFactory_ptr cfac, MiniMC::Support::Messager messager) : messager(std::move(messager)),cfactory(cfac) {} 
	MiniMC::Model::Program operator()  (MiniMC::Model::Program&& prgm) override;      
	
      private:
	MiniMC::Support::Messager messager;
	MiniMC::Model::ConstantFactory_ptr cfactory;
      };

    }
  }
}
		   

#endif 
