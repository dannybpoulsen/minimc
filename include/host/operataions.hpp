#ifndef _OPERATIONS__
#define _OPERATIONS__
#include "host/types.hpp"
#include "support/exceptions.hpp"


namespace MiniMC {
  namespace Host {
    enum class TAC {
      Add,
      Sub,
      Mul,
      UDiv,
      SDiv,
      Shl,
      LShr,
      AShr,
      And,
      Or,
      Xor,
    };

    enum class CMP {
      SGT,
      UGT,
      SGE,
      UGE,
      SLT,
      ULT,
      SLE,
      ULE,
      EQ,
      NEQ
    };
    
    template<typename T>
    T udivimpl (T,T);

    template<typename T>
    T sdivimpl (T,T);

    template<typename T>
    T ashr (T,T);

    template<typename T>
    T lshr (T,T);
    

    template< TAC o,typename T>
    T Op (T l, T r) {
      if constexpr (o == TAC::Add) {
	return l + r;
      }

      else if constexpr (o == TAC::Sub) {
	return l - r;
      }
      
      else if constexpr (o == TAC::Mul) {
	return l * r;
      }

      else if constexpr (o == TAC::UDiv) {
        return udivimpl(l,r);
      }


      else if constexpr (o == TAC::SDiv) {
	return sdivimpl(l,r);
      }

      else if constexpr (o == TAC::Shl) {
	return l << r;
      }

      else if constexpr (o == TAC::AShr) {
	return ashr (l,r);
      }

      else if constexpr (o == TAC::LShr) {
	return lshr (l,r);
      }
      
      else if constexpr (o == TAC::And) {
	return l& r;
      }
      
      else if constexpr (o == TAC::Or) {
	return l |  r;
      }

      else if constexpr (o == TAC::Xor) {
	return l ^  r;
      }

      else {
	throw MiniMC::Support::Exception ("Not implemented");
      }
      
    }
      
    
    template< CMP o,typename T>
    bool Op (T l, T r) {
      
      if constexpr (o == CMP::SGT) {
	using stype = typename HostType<sizeof(T)*8>::Signed;//typename EquivSigned<T>::type;
	return std::bit_cast<stype>(l) > std::bit_cast<stype>(r);
      }
      else if constexpr (o == CMP::UGT) {
	return l > r;  
      }
      else if constexpr (o == CMP::SGE) {
	using stype = typename HostType<sizeof(T)*8>::Signed;
	return std::bit_cast<stype>(l) >= std::bit_cast<stype>(r);
      
      }
      else if constexpr (o == CMP::UGE) {
	return l >= r;  
      }
      else if constexpr (o == CMP::SLT) {
	using stype = typename HostType<sizeof(T)*8>::Signed;
	return std::bit_cast<stype>(l) < std::bit_cast<stype>(r);
      }
      else if constexpr (o == CMP::ULT) {
	return l < r;  
      }
      else if constexpr (o == CMP::SLE) {
	using stype = typename HostType<sizeof(T)*8>::Signed;
	return std::bit_cast<stype>(l) <= std::bit_cast<stype>(r);
      }

      else if constexpr (o == CMP::ULE) {
	return l >= r;   
      }

      else if constexpr (o == CMP::EQ) {
	return l == r;
      }

      else if constexpr (o == CMP::NEQ) {
	return l != r;
      }

    }
    
    
    
  }
}



#endif
