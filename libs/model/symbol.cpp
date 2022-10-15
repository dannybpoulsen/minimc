#include "model/symbol.hpp"

#include <string>
#include <ostream>
#include <sstream>
#include <memory>

namespace MiniMC {
  namespace Model {
    struct Symbol::data {
      static const char delim {':'};
      data (std::string name) : parent(nullptr), name(std::move(name)) {}
      data (const std::shared_ptr<data>& p,std::string name ) : parent(p),name(std::move(name)) {} 
      
      std::ostream& output (std::ostream& os) const {
	if (parent) {
	  parent->output(os);
	  os << delim;
	}
	os  << name;
	return os;
      }
      
      std::shared_ptr<data> parent;
      std::string name;
      
    };

    std::string Symbol::getName () const {
      return _internal->name;
    }
      
    
    Symbol::Symbol (const std::string& s) {
      _internal = std::make_shared<data> (s);
    }

    Symbol::Symbol (const Symbol& s) : _internal(s._internal){
    }

    
    Symbol::Symbol (const Symbol& s, std::string name) {
      _internal = std::make_shared<data> (s._internal,name);
    }

    Symbol::Symbol (const std::shared_ptr<data>& d) : _internal(d) {}

    //assumtion hasPrefix() == true
    Symbol Symbol::prefix () const {
      return Symbol (_internal->parent);
    }

    bool Symbol::hasPrefix () const {
      return _internal->parent != nullptr;
    }
    
    
    Symbol::~Symbol () {}

    std::ostream& Symbol::output (std::ostream& os ) const {
      return _internal->output ( os);
    }
      
    
    Symbol Symbol::from_string (const  std::string& str) {
      std::stringstream stream {str};
      std::string inp;
      std::getline (stream,inp,data::delim);
      Symbol cur {inp};
      while (stream.good ())  {
	std::getline (stream,inp,data::delim);
	cur = Symbol {cur,inp};
      }

      return cur;
    }
    
  }
}
