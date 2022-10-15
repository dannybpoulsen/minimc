#ifndef _SYMBOL__
#define _SYMBOL__

#include <memory>
#include <string>
#include <ostream>
#include <sstream>

namespace MiniMC {
  namespace Model {
    class Symbol {
    public:
      static Symbol from_string (const  std::string& );
      explicit Symbol (const std::string&);
      Symbol ();
      
      Symbol (const Symbol&);
      Symbol (const Symbol&, std::string name);
      ~Symbol ();
      
      Symbol& operator=  (Symbol&&) = default;

      //assumtion hasPrefix() == true
      Symbol prefix () const;
      bool hasPrefix () const;
      std::ostream& output (std::ostream& ) const;  
      std::string to_string () const; 
      std::string getName () const;
      std::string getFullName () const {return to_string ();}

      
      
    private:
      struct data;
      std::shared_ptr<data> _internal;
      Symbol (const std::shared_ptr<data>&);
      
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
