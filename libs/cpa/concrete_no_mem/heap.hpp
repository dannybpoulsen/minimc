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
		   Invalid = 4,
		   Reusable = 8
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
	  if (offset+size > entry.size) {
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

	pointer_t findSpace (MiniMC::offset_t size) {
	  auto data = searchForReusable ();
	  return MiniMC::Support::makeHeapPointer (data.index,0);
	}
	
	pointer_t make_obj (MiniMC::offset_t size,pointer_t place) {
	  //auto data = searchForReusable ();
	  std::cerr << place << std::endl;
	  auto base = MiniMC::Support::getBase (place);
	  auto offset = MiniMC::Support::getOffset (place);
	  auto& entry = entries.at(base);
	  assert(offset == 0);
	  assert(entry.flags & Flags::Reusable);
	  entry.flags = 0;
	  entry.size = size;
	  entry.data = nullptr;
	  
	  if (size) {
	    assert (entries.size() <= std::numeric_limits<MiniMC::base_t>::max());
	    std::unique_ptr<MiniMC::uint8_t[]> ndata (new MiniMC::uint8_t[size]);
	    std::fill (ndata.get(),ndata.get()+size,0);
	    entry.data = MiniMC::Storage::getStorage().store (ndata,size);
	  }
	  return place;
	}

	
	
	template<class T>
	pointer_t make_obj_initialiser (MiniMC::offset_t size, const T& s, pointer_t place) {
	  assert(size>=sizeof(T));
	  auto base = MiniMC::Support::getBase (place);
	  auto offset = MiniMC::Support::getOffset (place);
	  auto& entry = entries.at(base);
	  assert(offset == 0);
	  assert(entry.flags & Flags::Reusable);
	  entry.flags = 0;
	  entry.size = size;
	  entry.data = nullptr;
	  
	  std::unique_ptr<MiniMC::uint8_t[]> ndata (new MiniMC::uint8_t[size]);
	  std::fill (ndata.get(),ndata.get()+size,0);
	  std::copy (reinterpret_cast<const MiniMC::uint8_t*> (&s),reinterpret_cast<const MiniMC::uint8_t*> (&s)+sizeof(T),ndata.get());
	  
	  entry.data = MiniMC::Storage::getStorage().store (ndata,size);
	  assert (entries.size() <= std::numeric_limits<MiniMC::base_t>::max());
	  return place;
	}

	void free_obj (const MiniMC::pointer_t& pointer,bool reusable = false) {
	  assert(MiniMC::Support::IsA<MiniMC::Support::PointerType::Heap>::check(pointer));
	  auto base = MiniMC::Support::getBase (pointer);
	  auto offset = MiniMC::Support::getOffset (pointer);
	  auto& entry = entries.at(base);
	  assert(offset == 0);
	  if (entry.flags & Flags::Invalid) {
	    throw MiniMC::Support::InvalidPointer ();
	  }
	  if (!reusable) {
	    entry.flags = Flags::Freed | Flags::Invalid;
	    
	  }
	  else {
	    entry.flags = Flags::Reusable | Flags::Freed | Flags::Invalid;
	  }
	  entry.size = 0;
	  entry.data = nullptr;
	}

	struct SearchData {
	  SearchData (HeapEntry* e, size_t index ) :entry(e),index(index) {}
	  HeapEntry* entry;
	  size_t index;
	};

	SearchData searchForReusable () {
	  size_t i = 0;
	  for (auto& e : entries) {
	    if (e.flags & Flags::Reusable) {
	      return SearchData (&e,i);
	    }
	    i++;
	  }
	  HeapEntry e;
	  e.flags = Flags::Reusable;
	  entries.push_back (e);
	  return SearchData (&entries.back(),i);
	}
	
	pointer_t extend_obj (const MiniMC::pointer_t& pointer,MiniMC::offset_t add) {
	  auto base = MiniMC::Support::getBase (pointer);
	  auto offset = MiniMC::Support::getOffset (pointer);
	  auto& entry = entries.at(base);
	  assert(offset == 0);
	  if (entry.flags & Flags::Invalid) {
	    throw MiniMC::Support::InvalidPointer ();
	  }
	  if (add) {
	    std::unique_ptr<MiniMC::uint8_t[]> res (new MiniMC::uint8_t[entry.size+add]);
	    std::copy(entry.data,entry.data+entry.size,res.get());
	    std::fill(res.get()+entry.size,res.get()+entry.size+add,0);
	    entry.data = MiniMC::Storage::getStorage().store (res,entry.size+add);
	    MiniMC::pointer_t respointer = MiniMC::Support::makeHeapPointer (base,entry.size);
	    entry.size+=add;
	    return respointer;
	  }
	  else {
	    return MiniMC::Support::makeHeapPointer (base,entry.size);
	  }
	  
	}

	std::unique_ptr<MiniMC::uint8_t[]> copy_out (const MiniMC::pointer_t& pointer) const  {
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
