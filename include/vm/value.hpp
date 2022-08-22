#ifndef _VM_VALUE__
#define _VM_VALUE__

#include <string>
#include <memory>
#include <variant>
#include "model/variables.hpp"
#include "util/array.hpp"
#include "support/exceptions.hpp"
#include "hash/hashing.hpp"


namespace MiniMC {
  namespace VMT {
    
    
    
    

    
    
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
      
      template<typename T>
      T& as () {	
	return std::get<T> (content);
      }

      template<class Func>
      auto visit (Func f) {
	return std::visit(f,content);
      }

      template<class Func>
      auto visit (Func f) const {
	return std::visit(f,content);
      }
      
      template<typename T>
      auto& as () const {	
	return std::get<T> (content);
      }
      
      template<typename T>
      bool is () const {
	return std::holds_alternative<T> (content);
      }
      
      auto hash () const {return std::hash<decltype(content)>{} (content);}

      auto& output (std::ostream& os) const {return std::visit([&](const auto& x) ->std::ostream&  { return os << x; }, content);}

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
