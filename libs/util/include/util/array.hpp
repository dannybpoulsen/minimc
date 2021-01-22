#ifndef _UTIL_ARRAY__
#define _UTIL_ARRAY__

#include <memory>
#include <iostream>
namespace MiniMC {
  namespace Util {
	class Array {
	public:
	  Array (size_t s) : buffer(new MiniMC::uint8_t[s]), size(s) {
		std::fill (buffer.get(),buffer.get()+size,0);
	  }
	  Array (const Array& a) : buffer (new MiniMC::uint8_t[a.size]),size(a.size) {
		std::copy (a.buffer.get(),a.buffer.get()+a.size,buffer.get());
	  }
	  
	  template<class T>
	  T read (std::size_t byte = 0) const {
		assert(byte+sizeof(T) <= size);
		T var;
		std::copy (buffer.get()+byte,buffer.get()+byte+size,&var);
		return var;
	  }
	  
	  template<class T>
	  void set (std::size_t byte, const T& t) {
		std::cerr << byte << " " << sizeof(T) << " " << size << std::endl;
		assert(byte+sizeof(T) <= size);
		std::copy (reinterpret_cast< const MiniMC::uint8_t*> (&t),reinterpret_cast<const MiniMC::uint8_t*> (&t)+sizeof(T),buffer.get()+byte);
	  }
	  
	  void set_block (std::size_t byte, std::size_t block_size, const MiniMC::uint8_t* block) {
		assert(byte+block_size <= size);
		std::copy (block,block+block_size,buffer.get()+byte);
	  }
	  
	  void get_block (std::size_t byte, std::size_t block_size, MiniMC::uint8_t* block) {
		assert(byte+block_size <= size);
		std::copy (buffer.get()+byte,buffer.get()+byte+block_size,block);
	  }
	  
	  auto getSize () const {return size;}
	  
	private:
	  std::unique_ptr<MiniMC::uint8_t[]> buffer;
	  std::size_t size;
	};
  }	
}

#endif
