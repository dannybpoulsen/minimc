#ifndef _SYMBOL__
#define _SYMBOL__

#include <memory>
#include <string>
#include <ostream>
#include <sstream>
#include <list>
#include <variant>
#include <generator>
#include <optional>

#include "minimc/hash/hashing.hpp"
#include "minimc/io/ostream.hpp"

namespace MiniMC {
  namespace Model {

    class Register;
    class Function;
    class Location;
    class HeapBlock;
    
    
    using Register_wptr = std::weak_ptr<Register>;
    using Register_ptr = std::shared_ptr<Register>;
    
    using Function_ptr = std::shared_ptr<Function>;
    using Location_wptr = std::weak_ptr<Location>;
    using HeapBlock_wptr = std::weak_ptr<HeapBlock>;
    
    
    using UserData = std::variant<Register_ptr,
				  Function_ptr,
				  Location_wptr,
				  HeapBlock_wptr,
				  std::monostate>;
    
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
      MiniMC::IO::ostream& output (MiniMC::IO::ostream& ) const;
      std::string to_string () const; 
      std::string getName () const;
      std::string getFullName () const {return to_string ();}
      void setUserData (UserData data);
      const UserData& getUserData () const ;
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
      Frame (const std::string& ="");
      ~Frame ();
      Frame (const Frame& f) = default;
      Frame (Frame&& f) = default;
      
      Frame open (const std::string& s);
      Frame create (const std::string& s);
      
      Frame close ();
      std::optional<Symbol> resolve (const std::string&) const ;
      std::optional<Symbol>resolveQualified (const std::string&) const ;
      Symbol makeSymbol (const std::string& s);
      
      Symbol makeFresh (const std::string& = "fresh");
      Frame& operator= (const Frame& f) = default;
      std::generator<Symbol> symbols () const ;

      //Stupid naming
      std::generator<Symbol> local_symbols () const ;
      std::generator<Symbol> local_and_parent_symbols () const ;
      
      bool hasSymbol (const Symbol& ) const;

      std::generator<std::tuple<Symbol,MiniMC::Model::Register_ptr>> local_registers () const ;
      std::size_t numberOfRegisters () const;
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
