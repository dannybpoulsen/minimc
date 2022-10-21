#include "model/symbol.hpp"

#include <string>
#include <ostream>
#include <sstream>
#include <iostream>
#include <memory>
#include <list>
#include <cassert>
#include <algorithm>

namespace MiniMC {
  namespace Model {
    struct Symbol::data {
      static const char delim {':'};
      data () {}
      data (std::string name) : names({name}) {
      }
      
      data (std::list<std::string> s) : names(std::move(s)) {}
      
      std::ostream& output (std::ostream& os) const {
	bool started = false;
	std::for_each (names.begin (), names.end (), [&started,&os](const auto& s) {
	  if (started) {
	    os << delim;
	  }
	  started = true;
	  os << s;  
	    
	});
	
	return os;
      }
      
      std::list<std::string> names;
    };

    std::string Symbol::getName () const {
      if (_internal->names.size () > 0)
	return _internal->names.back ();
      else
	return "";
    }

    Symbol::Symbol () {
      _internal = std::make_unique<data> ();
    }
      
    
    Symbol::Symbol (const std::string& s) {
      _internal = std::make_unique<data> (s);
    }

    Symbol::Symbol (const Symbol& s) {
      _internal = std::make_unique<data> (s._internal->names);
    }


    Symbol::Symbol (const Symbol& pref, Symbol&& end) {
      _internal = std::make_unique<data> ();
      std::copy (pref._internal->names.begin(),pref._internal->names.end(),std::back_inserter (_internal->names));
      std::copy (end._internal->names.begin(),end._internal->names.end(),std::back_inserter (_internal->names));
      
    }
    
    Symbol::Symbol (const Symbol& s, std::string name) {
      _internal = std::make_unique<data> ();
      std::copy (s._internal->names.begin(),s._internal->names.end(),std::back_inserter (_internal->names));
      _internal->names.push_back (name);
    }

    Symbol::Symbol (std::unique_ptr<data>&& data) : _internal(std::move(data)) {
     
    }

    Symbol::Symbol(const std::list<std::string> list) {
      _internal = std::make_unique<data> (list);
    }
    
    
    //assumtion hasPrefix() == true
    Symbol Symbol::prefix () const {
      std::list<std::string> names;
      std::copy (_internal->names.begin (),_internal->names.end ()--,std::back_inserter (names));
      return Symbol (std::make_unique<data> (names));
    }

    bool Symbol::hasPrefix () const {
      return _internal->names.size () >  1;	
    }

    bool Symbol::isRoot () const {
      return _internal->names.size () == 0;
    }
    
    Symbol::~Symbol () {}
    
    std::ostream& Symbol::output (std::ostream& os ) const {
      return _internal->output ( os);
    }
    
    
    Symbol Symbol::from_string (const  std::string& str) {
      std::list<std::string> names;
      auto inserter = std::back_inserter (names);
      std::stringstream stream {str};
      std::string inp;
      std::getline (stream,inp,data::delim);
      inserter = inp;
      while (stream.good ())  {
	std::getline (stream,inp,data::delim);
	inserter = inp;
      }

      return Symbol {std::make_unique<data> (std::move(names))};
    }
    
  }
}
