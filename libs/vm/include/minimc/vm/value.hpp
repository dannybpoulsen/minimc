#ifndef _VM_VALUE__
#define _VM_VALUE__

#include <string>
#include <memory>
#include <type_traits>
#include <variant>
#include "minimc/model/variables.hpp"
#include "minimc/model/array.hpp"
#include "minimc/support/exceptions.hpp"
#include "minimc/hash/hashing.hpp"


namespace MiniMC {
  namespace VMT {

    template<class T, class... Ts>
    concept is_same = 
    (... && std::is_same<T, Ts>::value);
    
    template<typename Int8,typename Int16,typename Int32,typename Int64, typename PointerT, typename Pointer32T, typename BoolT,typename Ag>
    struct GenericVal {
      using I8 = Int8;
      using I16 = Int16;
      using I32 = Int32;
      using I64 = Int64;
      using Pointer = PointerT;
      using Pointer32 = Pointer32T;
      using Bool = BoolT;
      using Aggregate = Ag;
      
      
      GenericVal () : content(BoolT{}) {} 
      GenericVal (I8 val) : content(val) {}
      GenericVal (I16 val) : content(val) {}
      GenericVal (I32 val) : content(val) {}
      GenericVal (I64 val) : content(val) {}
      GenericVal (Pointer val) : content(val) {}
      GenericVal (Pointer32 val) : content(val) {}
      GenericVal (Bool val) : content(val) {}
      GenericVal (Aggregate ag) : content(ag) {}
      

      template<class Func, class... Values> requires (... && std::is_same_v<GenericVal,Values>)
      static auto visit (Func f,Values... values)  {
	return std::visit (f,values.content...);
      }
    
      auto hash () const {return std::hash<decltype(content)>{} (content);}

      auto& output (std::ostream& os) const {return std::visit([&os](const auto& x) ->std::ostream&  { return os << x; }, content);}

      bool operator== (const GenericVal& oth) const {return oth.content == content;} 
      
    private:
      
      std::variant<I8,I16,I32,I64,Pointer,Pointer32,Bool,Ag> content;
    };

    
    template<class G>
    concept Outputtable = requires (std::ostream& os, const G&g) {g.output (os);};
    
    template<Outputtable G> 
    inline std::ostream& operator<< (std::ostream&  os, const G& val) {
      return val.output (os);
    }
    
  }
  
} // namespace MiniMC

#endif
