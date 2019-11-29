#ifndef _HEAP__
#define _HEAP__

#include "hash/hashing.hpp"
#include "support/types.hpp"
#include "support/pointer.hpp"
#include "support/exceptions.hpp"
#include "storage/BlobStorage.hpp"
#include "register.hpp"

namespace MiniMC {
  namespace CPA {
    namespace ConcreteNoMem {
      enum  Flags {
		   Initialised = 1,
		   Freed = 2,
		   Invalid = 4
      };
      struct __attribute__ ((packed)) HeapEntry {
	MiniMC::uint8_t flags = Flags::Invalid;
	MiniMC::offset_t size = 0;
	MiniMC::uint8_t* data = nullptr;
      };
      class Heap {
      public:
	InRegister read (const MiniMC::pointer_t& pointer, MiniMC::offset_t size) {
	  assert(MiniMC::Support::IsA<MiniMC::Support::PointerType::Heap>::check(pointer));
	  auto base = MiniMC::Support::getBase (pointer);
	  auto offset = MiniMC::Support::getOffset (pointer);
	  auto& entry = entries.at(base);
	  if (entry.flags & Flags::Invalid) {
	    throw MiniMC::Support::InvalidPointer ();
	  }
	  if (offset+size > size) {
	    throw MiniMC::Support::BufferOverflow ();
	  }
	  return InRegister (entry.data+offset,size);
	}

	template<class Register>
	void write (const MiniMC::pointer_t& pointer, const Register& reg) {
	  assert(MiniMC::Support::IsA<MiniMC::Support::PointerType::Heap>::check(pointer));
	  auto base = MiniMC::Support::getBase (pointer);
	  auto offset = MiniMC::Support::getOffset (pointer);
	  auto& entry = entries.at(base);
	  if (entry.flags & Flags::Invalid) {
	    throw MiniMC::Support::InvalidPointer ();
	  }
	  if (offset+reg.getSize() > entry.size) {
	    throw MiniMC::Support::BufferOverflow ();
	  }

	  std::unique_ptr<MiniMC::uint8_t[]> ndata (new MiniMC::uint8_t[entry.size]);
	  std::copy (entry.data,entry.data+entry.size,ndata.get());
	  auto start = reinterpret_cast<const MiniMC::uint8_t*> (reg.getMem()); 
	  std::copy (start,start+reg.getSize(),ndata.get()+offset);
	  entry.data = MiniMC::Storage::getStorage().store (ndata,entry.size);
	  entry.flags = entry.flags | Flags::Initialised;
	  
	}

	pointer_t make_obj (MiniMC::offset_t size) {
	  std::unique_ptr<MiniMC::uint8_t[]> ndata (new MiniMC::uint8_t[size]);
	  std::fill (ndata.get(),ndata.get()+size,0);
	  HeapEntry entry;
	  entry.flags = 0;
	  entry.size = size;
	  entry.data = MiniMC::Storage::getStorage().store (ndata,size);
	  assert (entries.size() <= std::numeric_limits<MiniMC::base_t>::max());
	  pointer_t ptr = MiniMC::Support::makeHeapPointer (entries.size(),0);
	  entries.push_back (entry);
	  return ptr;
	}

	void free_obj (const MiniMC::pointer_t& pointer) {
	  assert(MiniMC::Support::IsA<MiniMC::Support::PointerType::Heap>::check(pointer));
	  auto base = MiniMC::Support::getBase (pointer);
	  auto offset = MiniMC::Support::getOffset (pointer);
	  auto& entry = entries.at(base);
	  assert(offset == 0);
	  if (entry.flags & Flags::Invalid) {
	    throw MiniMC::Support::InvalidPointer ();
	  }
	  
	  entry.flags = Flags::Freed | Flags::Invalid;
	  entry.size = 0;
	  entry.data = nullptr;
	  
	}

	pointer_t extend_obj (const MiniMC::pointer_t& pointer,MiniMC::offset_t add) {
	  auto base = MiniMC::Support::getBase (pointer);
	  auto offset = MiniMC::Support::getOffset (pointer);
	  auto& entry = entries.at(base);
	  assert(offset == 0);
	  if (entry.flags & Flags::Invalid) {
	    throw MiniMC::Support::InvalidPointer ();
	  }

	  std::unique_ptr<MiniMC::uint8_t[]> res (new MiniMC::uint8_t[entry.size+add]);
	  std::copy(entry.data,entry.data+entry.size,res.get());
	  std::fill(res.get()+entry.size,res.get()+entry.size+add,0);
	  entry.data = MiniMC::Storage::getStorage().store (res,entry.size+add);
	  MiniMC::pointer_t respointer = MiniMC::Support::makeHeapPointer (base,entry.size);
	  entry.size+=add;
	  return respointer;
	  
	}

	std::unique_ptr<MiniMC::uint8_t[]> copy_out (const MiniMC::pointer_t& pointer) {
	  assert(MiniMC::Support::IsA<MiniMC::Support::PointerType::Heap>::check(pointer));
	  auto base = MiniMC::Support::getBase (pointer);
	  auto offset = MiniMC::Support::getOffset (pointer);
	  auto& entry = entries.at(base);
	  assert(offset == 0);
	  if (entry.flags & Flags::Invalid) {
	    throw MiniMC::Support::InvalidPointer ();
	    
	  }

	  std::unique_ptr<MiniMC::uint8_t[]> res (new MiniMC::uint8_t[entry.size]);
	  std::copy(entry.data,entry.data+entry.size,res.get());
	  return res;
	  
	}

	
	void writeRaw (const MiniMC::pointer_t& pointer, std::unique_ptr<MiniMC::uint8_t[]>& data) {
	  assert(MiniMC::Support::IsA<MiniMC::Support::PointerType::Heap>::check(pointer));
	  auto base = MiniMC::Support::getBase (pointer);
	  auto offset = MiniMC::Support::getOffset (pointer);
	  auto& entry = entries.at(base);
	  assert(offset == 0);
	  if (entry.flags & Flags::Invalid) {
	    throw MiniMC::Support::InvalidPointer ();
	    
	  }
	  entry.data  = MiniMC::Storage::getStorage().store (data,entry.size);
	  entry.flags = entry.flags | Flags::Initialised;
	}

	
	MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t seed) const {
	  return MiniMC::Hash::Hash (entries.data(),entries.size(),seed);
	}
	
      private:
	std::vector<HeapEntry> entries;
      };
      
    }
  }
}


#endif
