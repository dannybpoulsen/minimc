#ifndef _MODEL__FLAGS__
#define _MODEL__FLAGS__

namespace MiniMC {
  namespace Model {
    template<class T>
    struct FlagSet {
      using V = std::underlying_type_t<T>;
      FlagSet () : value(0) {}
      FlagSet (const T& t) : value(static_cast<V> (t)) {}
      FlagSet (const FlagSet& oth) :value(oth.value) {}
      
      auto& operator|= (const T& t) {
	value |= static_cast<V> (t);
	return *this;
      }

      auto& operator= (const FlagSet<T>& oth) {value = oth.value; return *this;} 
      
      bool isSet (const T& t) const {
	return value & static_cast<V> (t);
      }

      
      private:
      V value;
    };

  }
}

#endif
