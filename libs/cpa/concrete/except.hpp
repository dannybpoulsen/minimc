
#ifndef _EXCEPT__
#define _EXCEPT__

#include "support/localisation.hpp"
#include "support/exceptions.hpp"

namespace MiniMC {
  namespace CPA {
    namespace Concrete {
	  template<MiniMC::Model::InstructionCode c>
      class NotImplemented : public MiniMC::Support::Exception {
      public:
		NotImplemented () : MiniMC::Support::Exception (MiniMC::Support::Localiser{"Instruction '%1%' not implemented for this CPA"}.format (c)) {}
      };
      
      class BufferOverflow : public MiniMC::Support::Exception {
      public:
		BufferOverflow () : MiniMC::Support::Exception (MiniMC::Support::Localiser{"BufferOverflow detected"}.format ()) {}
      };

	  class BufferOverread : public MiniMC::Support::Exception {
      public:
		BufferOverread () : MiniMC::Support::Exception (MiniMC::Support::Localiser{"BufferOverread detected"}.format ()) {}
      };
	  
	  
	  class InvalidFree : public MiniMC::Support::Exception {
      public:
		InvalidFree () : MiniMC::Support::Exception (MiniMC::Support::Localiser{"Invalid free detected"}.format ()) {}
      };

	  class InvalidExtend : public MiniMC::Support::Exception {
      public:
		InvalidExtend () : MiniMC::Support::Exception (MiniMC::Support::Localiser{"Invalid extend detected"}.format ()) {}
      };
	  
	  
	  
	}
  }
}


#endif
