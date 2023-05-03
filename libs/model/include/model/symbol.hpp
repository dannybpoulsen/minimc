#ifndef _SYMBOL__
#define _SYMBOL__

#include <memory>
#include <string>
#include <ostream>
#include <sstream>
#include <list>

#include "hash/hashing.hpp"

namespace MiniMC {
  namespace Model {
    class Symbol {
    public:
      friend class Frame;
      static Symbol from_string (const  std::string& );
      Symbol ();
      Symbol (const Symbol&);
      
      ~Symbol ();
      
      Symbol& operator=  (Symbol&&) = default;
      Symbol& operator=  (const Symbol&);
      
      //assumtion hasPrefix() == true
      Symbol prefix () const;
      bool hasPrefix () const;
      bool isRoot () const;
      std::ostream& output (std::ostream& ) const;  
      std::string to_string () const; 
      std::string getName () const;
      std::string getFullName () const {return to_string ();}

      MiniMC::Hash::hash_t hash () const;
      bool operator== (const Symbol& d) const ;
      
      struct data;
    protected:
      explicit Symbol (const std::string&);
      Symbol (const Symbol&, std::string name);
      Symbol (const Symbol&, Symbol&&);
      
    private:
      std::shared_ptr<data> _internal;
      Symbol (std::shared_ptr<data>);
    };
    
    
    class Frame {
    public:
      Frame ();
      ~Frame ();
      Frame (const Frame& f) = default;
      Frame (Frame&& f) = default;
      
      Frame open (const std::string& s);
      Frame create (const std::string& s);
      
      Frame close ();
      bool resolve (const std::string&, Symbol& s);
      bool resolveQualified (const std::string&, Symbol& s);
      
      Symbol makeSymbol (const std::string& s);
      Symbol makeFresh (const std::string& = "fresh");
      
      
    private:
      struct Internal;
      Frame (std::shared_ptr<Internal>&& s) : _internal(std::move(s)) {}
      Frame (const std::shared_ptr<Internal>& s) : _internal(s) {}
      
      
      std::shared_ptr<Internal> _internal;

    };
    
    
    inline std::ostream& operator<< (std::ostream& os, const Symbol& symb) {
      return symb.output (os);
    }


    inline std::string Symbol::to_string () const  {
      std::stringstream str;
      str << *this;
      return str.str ();
    }

    template<class T>
    using SymbolTable = std::unordered_map<Symbol, T>;
    
  }
}


#endif
