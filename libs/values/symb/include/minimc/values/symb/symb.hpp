#ifndef _SYMB_VALUES__
#define _SYMB_VALUES__

#include "minimc/model/types.hpp"
#include "minimc/hash/hashing.hpp"
#include "minimc/vm/value.hpp"

namespace MiniMC {
  namespace Values {
    namespace Symb {
      template<MiniMC::Model::TypeID> 
      class TValue : public MiniMC::Hash::RandomHash  {
      public:
	TValue (MiniMC::Model::Value_ptr&& v) : value(std::move(v)) {}
	auto& getEpr () const {return value;}
	std::ostream& output(std::ostream& os) const {
	  return value->output (os);
	}

	MiniMC::VMT::TriBool boolState () const {return MiniMC::VMT::TriBool::Unk;}
	
	
      private:
	MiniMC::Model::Value_ptr value;
      };

      template<MiniMC::Model::TypeID id>
      inline std::ostream& operator<<(std::ostream& os, const TValue<id>& val) { return val.output(os); }

      using Value = MiniMC::VMT::GenericVal<TValue<MiniMC::Model::TypeID::I8>,
					    TValue<MiniMC::Model::TypeID::I8>,
					    TValue<MiniMC::Model::TypeID::I8>,
					    TValue<MiniMC::Model::TypeID::I8>,
					    TValue<MiniMC::Model::TypeID::Pointer>,
					    TValue<MiniMC::Model::TypeID::Pointer32>,
					    TValue<MiniMC::Model::TypeID::Bool>,
					    TValue<MiniMC::Model::TypeID::Aggregate>,
					    TValue<MiniMC::Model::TypeID::Memory>
					    >;
      
      
    }
  }
}

#endif 
