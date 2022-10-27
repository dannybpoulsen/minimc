#ifndef _SYMBOL__
#define _SYMBOL__

#include <memory>
#include <string>
#include <ostream>
#include <sstream>
#include <list>

namespace MiniMC {
  namespace Model {
    class Symbol {
    public:
      static Symbol from_string (const  std::string& );
      explicit Symbol (const std::string&);
      Symbol ();
      
      Symbol (const Symbol&);
      Symbol (const Symbol&, std::string name);
      Symbol (const Symbol&, Symbol&&);
      Symbol(const std::list<std::string> list);
      
      ~Symbol ();
      
      Symbol& operator=  (Symbol&&) = default;

      //assumtion hasPrefix() == true
      Symbol prefix () const;
      bool hasPrefix () const;
      bool isRoot () const;
      std::ostream& output (std::ostream& ) const;  
      std::string to_string () const; 
      std::string getName () const;
      std::string getFullName () const {return to_string ();}

      
      
          private:
      struct data;
      std::unique_ptr<data> _internal;
      Symbol (std::unique_ptr<data>&&);
      
    };

    inline std::ostream& operator<< (std::ostream& os, const Symbol& symb) {
      return symb.output (os);
    }


    inline std::string Symbol::to_string () const  {
      std::stringstream str;
      str << *this;
      return str.str ();
    }
      
    
  }
}


#endif
