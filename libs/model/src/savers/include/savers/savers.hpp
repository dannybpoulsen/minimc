#ifndef _SAVER__
#define _SAVER__

#include <ostream>

#include "model/cfg.hpp"

namespace MiniMC {
  namespace Savers {
	/**
	 * The possible model loaders. 
	 **/
	enum class Type {
	  JSON /**< LLVM Model */
	};

	struct BaseSaveOptions {
	  std::ostream* writeTo;
	};
	
	template<Type>
	struct OptionsSave {
	  using Opt = BaseSaveOptions;
	};

	
	template<Type t,class Arg = typename OptionsSave<t>::Opt>
	void saveModel (const MiniMC::Model::Program_ptr&, Arg saveOptions);
	
  }
}


#endif
