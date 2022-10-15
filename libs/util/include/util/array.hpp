#ifndef _UTIL_ARRAY__
#define _UTIL_ARRAY__

#include "hash/hashing.hpp"
#include "support/binary_encode.hpp"
#include <memory>
#include <ostream>

namespace MiniMC {
  namespace Util {
    class Array {
    public:
      Array() : buffer(nullptr), size(0) {
      }
      Array(size_t s) : buffer(new MiniMC::BV8[s]), size(s) {
        std::fill(buffer.get(), buffer.get() + size, 0);
      }

      Array(size_t s, const MiniMC::BV8* buffer) : buffer(new MiniMC::BV8[s]), size(s) {
        std::copy(buffer, buffer + size, this->buffer.get());
      }

      Array(const MiniMC::BV8* begin,const MiniMC::BV8* end) : buffer(new MiniMC::BV8[end-begin]), size(end-begin) {
        std::copy(begin, end, this->buffer.get());
      }
      
      Array(const Array& a) {
        *this = a;
      }

      Array& operator=(const Array& a) {
        //assert(getSize() == a.getSize ());
        buffer.reset(new MiniMC::BV8[a.size]);
        size = a.size;
        std::copy(a.buffer.get(), a.buffer.get() + a.size, buffer.get());
        return *this;
      }

      template <class T>
      T read(std::size_t byte = 0) const {
        assert(buffer);
        assert(byte + sizeof(T) <= size);
        T var;
        std::copy(buffer.get() + byte, buffer.get() + byte + sizeof(T), reinterpret_cast<MiniMC::BV8*>(&var));
        return var;
      }

      template <class T>
      void set(std::size_t byte, const T& t) {
        assert(byte + sizeof(T) <= size);
        std::copy(reinterpret_cast<const MiniMC::BV8*>(&t), reinterpret_cast<const MiniMC::BV8*>(&t) + sizeof(T), buffer.get() + byte);
      }

      void set (const Array& oth, std::size_t byte) {
	std::copy (oth.buffer.get(),oth.buffer.get()+oth.getSize(),buffer.get()+byte);
      }
      
      void set_block(std::size_t byte, std::size_t block_size, const MiniMC::BV8* block) {
        assert(byte + block_size <= size);
        std::copy(block, block + block_size, buffer.get() + byte);
      }

      void get_block(std::size_t byte, std::size_t block_size, MiniMC::BV8* block)  const {
        assert(byte + block_size <= size);
        std::copy(buffer.get() + byte, buffer.get() + byte + block_size, block);
      }

      const MiniMC::BV8* get_direct_access() const {
        return buffer.get();
      }

      MiniMC::BV8* get_direct_access()  {
        return buffer.get();
      }

      std::size_t getSize() const { return size; }

      std::ostream& output(std::ostream& os) const {
        MiniMC::Support::STDEncode encoder;
        return os << encoder.encode(reinterpret_cast<const char*>(buffer.get()), size);
      }

      MiniMC::Hash::hash_t hash() const {
	MiniMC::Hash::hash_t s{0};
	return MiniMC::Hash::Hash(buffer.get(), size, s);
      }

    private:
      std::unique_ptr<MiniMC::BV8[]> buffer;
      std::size_t size;
    };

    inline std::ostream& operator<<(std::ostream& os, const Array& vec) {
      return vec.output(os);
    }
    
  } // namespace Util
} // namespace MiniMC

#endif
