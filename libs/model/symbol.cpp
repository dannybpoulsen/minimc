#include "model/symbol.hpp"

#include <string>
#include <ostream>
#include <memory>

namespace MiniMC {
  namespace Model {
    struct Symbol::data {
      data (std::string name) : parent(nullptr), name(std::move(name)) {}
      data (const std::shared_ptr<data>& p,std::string name ) : parent(p),name(std::move(name)) {} 
      
      std::ostream& output (std::ostream& os) const {
	if (parent) {
	  parent->output(os);
	  os << ":";
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

    Symbol Symbol::prefix () const {
      return Symbol (_internal->parent,"");
    }

    
    
    
    Symbol::~Symbol () {}

    std::ostream& Symbol::output (std::ostream& os ) const {
      return _internal->output ( os);
    }
      
    
  }
}
