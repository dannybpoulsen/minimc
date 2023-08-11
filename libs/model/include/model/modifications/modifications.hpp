#ifndef _MODIFICATIONS__
#define _MODIFICATIONS__

#include "support/pipes.hpp"


namespace MiniMC {
  namespace Model {
    namespace Modifications {
      using ProgramModifier = MiniMC::Support::Transformer<MiniMC::Model::Program>;
      using ProgramManager  = MiniMC::Support::TransformManager<MiniMC::Model::Program>;
      
    }
  }
}

#include "model/modifications/func_inliner.hpp"
#include "model/modifications/loops.hpp"
#include "model/modifications/removephi.hpp"
#include "model/modifications/splitasserts.hpp"
#include "model/modifications/expandnondet.hpp"


#endif 
