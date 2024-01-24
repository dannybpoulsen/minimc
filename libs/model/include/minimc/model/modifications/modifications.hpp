#ifndef _MODIFICATIONS__
#define _MODIFICATIONS__

#include "minimc/model/cfg.hpp"
#include "minimc/support/pipes.hpp"


namespace MiniMC {
  namespace Model {
    namespace Modifications {
      using ProgramModifier = MiniMC::Support::Transformer<MiniMC::Model::Program>;
      using ProgramManager  = MiniMC::Support::TransformManager<MiniMC::Model::Program>;
      
    }
  }
}

#include "minimc/model/modifications/func_inliner.hpp"
#include "minimc/model/modifications/loops.hpp"
#include "minimc/model/modifications/removephi.hpp"
#include "minimc/model/modifications/splitasserts.hpp"
#include "minimc/model/modifications/expandnondet.hpp"


#endif 
