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

	}
  }
}
