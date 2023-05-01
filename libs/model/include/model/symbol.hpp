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

    class Frame;
    using Frame_ptr = std::shared_ptr<Frame>;
    
    class Frame {
    public:
      static Frame_ptr RootFrame () { return  Frame_ptr{new Frame (MiniMC::Model::Symbol {})};}
      
      Frame_ptr open (const std::string& s)  {
	return Frame_ptr{new Frame (Symbol{symb,s},Frame_ptr(this))};
      }

      Frame_ptr close () const {
	return parent;
      }
      
      Symbol makeSymbol (const std::string& s) {return Symbol{symb,s};}
      
    private:
      Frame (Symbol symb) : symb(symb),parent(nullptr) {}
      Frame (Symbol symb,Frame_ptr f) : symb(symb),parent(std::move(f)) {}
      Symbol symb;
      Frame_ptr parent;
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
