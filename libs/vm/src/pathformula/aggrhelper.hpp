#ifndef _SMT_PATH__AGGRHELPER
#define _SMT_PATH__AGGRHELPER

#include "vm/pathformula/value.hpp"
#include "smt/smtconstruction.hpp"
#include "smt/builder.hpp"
#include <bit>
#include <type_traits>


namespace MiniMC {
  namespace VMT {
    namespace Pathformula {
      enum class LoadType {
	Straight, //No conversion whatsoever,
	Little, //Load using little-endian interpretation
	Big, //Load using Big endian interpretation
      };

      consteval LoadType NativeLoad () {
	if constexpr (std::endian::native == std::endian::big) {
	  return LoadType::Big;
	}

	else if constexpr (std::endian::native == std::endian::little) {
	  return LoadType::Little;
	}
	else {
	  return LoadType::Straight;
	}
      }
      
      struct BVHelper {
	BVHelper (SMTLib::TermBuilder& b, const SMTLib::Term_ptr& ptr,std::size_t size) : builder(b),value(ptr),termsize(size) {}
	template<LoadType lt>
	auto extractBytes (std::size_t offset, std::size_t size) {
	  if constexpr (lt == LoadType::Straight) {
	    auto lowbit = offset*8;
	    auto highbit = lowbit+size*8-1;
	    return builder.buildTerm(SMTLib::Ops::Extract, {value}, {highbit,lowbit}); 
	  }
	  else  {
	   MiniMC::Util::Chainer<SMTLib::Ops::Concat> chainer{&builder};
	   for (size_t  i = offset; i < size; ++i) {
	     auto lowbit = i*8;
	     auto highbit = lowbit+7;
	     auto nterm = builder.buildTerm(SMTLib::Ops::Extract, {value}, {highbit,lowbit}); 
	     if constexpr (lt == LoadType::Little) {
	       chainer >> nterm;
	     }
	     else if constexpr (lt == LoadType::Big) {
	       chainer << nterm;
	     }
	   }
	   return chainer.getTerm ();
	  }
	}

	template<LoadType lt>
	auto storeBytes (std::size_t offset, const SMTLib::Term_ptr& insertee,std::size_t insert_size) {
	  MiniMC::Util::Chainer<SMTLib::Ops::Concat> chainer{&builder};
	  auto ins_converted = BVHelper{builder,insertee,insert_size}.extractBytes<lt> (0,insert_size);
	    
	  //Insert at beginning
	  if (offset == 0) {
	    
	    auto extracted = extractBytes<LoadType::Straight> (insert_size,termsize-insert_size);   return  (chainer <<extracted << ins_converted).getTerm ();
	    
	  }

	  // insert at the very end
	  else if (offset + insert_size == termsize) {
	    auto extracted = extractBytes<LoadType::Straight> (0,offset);
	    return  (chainer << ins_converted << extracted).getTerm ();
	    
	  }
	  // Insert in the middle
	  else  {
	    auto init_extract = extractBytes<LoadType::Straight> (0,offset);
	    auto last_extract =  extractBytes<LoadType::Straight> (offset+insert_size,termsize-offset-insert_size);	    
            chainer << last_extract << ins_converted << init_extract;
            return chainer.getTerm ();
          }
	}
      private:
	SMTLib::TermBuilder& builder;
	const SMTLib::Term_ptr& value;
	std::size_t termsize;
      };
    }
  }
} // namespace MiniMC


#endif
