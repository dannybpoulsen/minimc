#ifndef _MODIFICATIAONS__EXPANDNONDET__
#define _MODIFICATIAONS__EXPANDNONDET__

#include "model/cfg.hpp"
#include "model/modifications/modifications.hpp"
#include "support/feedback.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      class NonDetExpander : public ProgramModifier {
      public:
	NonDetExpander (MiniMC::Support::Messager messager) : messager(std::move(messager)) {} 
	MiniMC::Model::Program operator()  (MiniMC::Model::Program&& prgm) override;      
	
      private:
	MiniMC::Support::Messager messager;
      };

    }
  }
}
		   

#endif 
