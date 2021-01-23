#ifndef _UTIL_ARRAY__
#define _UTIL_ARRAY__

#include <memory>
#include <ostream>
#include "support/binary_encode.hpp"


namespace MiniMC {
  namespace Util {
	class Array {
	public:
	  Array () : buffer(nullptr),size(0) {
		
	  }
	  Array (size_t s) : buffer(new MiniMC::uint8_t[s]), size(s) {
		std::fill (buffer.get(),buffer.get()+size,0);
	  }
	  
	  Array (const Array& a) : buffer (new MiniMC::uint8_t[a.size]),size(a.size) {
		std::copy (a.buffer.get(),a.buffer.get()+a.size,buffer.get());
	  }

	  Array& operator= (const Array& a) {
		//assert(getSize() == a.getSize ());
		buffer.reset (new MiniMC::uint8_t[a.size]);
		size = a.size;
		std::copy (a.buffer.get(),a.buffer.get()+a.size,buffer.get());
		return *this;
	  }
	  
	  template<class T>
	  T read (std::size_t byte = 0) const {
		assert(buffer);
		assert(byte+sizeof(T) <= size);
		T var;
		std::copy (buffer.get()+byte,buffer.get()+byte+sizeof(T),reinterpret_cast<MiniMC::uint8_t*> (&var));
		return var;
	  }
	  
	  template<class T>
	  void set (std::size_t byte, const T& t) {
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
	  
	  std::size_t getSize () const {return size;}

	  std::ostream& output (std::ostream& os) const  {
		MiniMC::Support::Base64Encode encoder;
		return os << encoder.encode (reinterpret_cast<const char*> (buffer.get()),size);
	  }
	  
	private:
	  std::unique_ptr<MiniMC::uint8_t[]> buffer;
	  std::size_t size;
	};

	inline std::ostream& operator<< (std::ostream& os, const Array& vec) {
	  return vec.output (os);
	}
	
  }	
}

#endif
