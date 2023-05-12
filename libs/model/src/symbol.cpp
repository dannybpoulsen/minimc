#include "model/symbol.hpp"
#include "support/exceptions.hpp"
#include "hash/hashing.hpp"

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
      data (std::shared_ptr<data> p = nullptr) : parent(std::move(p)) {}
      data (std::string s,  std::shared_ptr<data> p = nullptr) : parent(std::move(p)),name(s) {}
      
      std::ostream& output (std::ostream& os) const {
	if (parent && !parent->isRoot ()) {
	  
	  return parent->output (os) << delim << name;
	  
	}
	else
	  return os << name;
      }

      bool isRoot () const {
	return parent == nullptr && name == "";
      }

      MiniMC::Hash::hash_t hash () const {
	if (!_hash) {
	  MiniMC::Hash::Hasher hasher;
	  hasher << name;
	  if (parent)
	    hasher << *parent;
	  _hash = hasher;
	}
	return _hash;
      }
      
      bool operator== (const data& d) const {
	return name == d.name && parent == d.parent;
      } 

      
      std::shared_ptr<data> parent;
      std::string name;
      mutable MiniMC::Hash::hash_t _hash{0};
    };

    
    
    std::string Symbol::getName () const {
	return _internal->name;
    }

    Symbol::Symbol () {
      _internal = std::make_shared<data> ();
    }
      
    
    Symbol::Symbol (const std::string& s) {
      _internal = std::make_shared<data> (s);
    }

    Symbol::Symbol (const Symbol& s) {
      _internal = s._internal;
    }

    Symbol& Symbol::operator= (const Symbol& s) {
      _internal = s._internal;
      return *this;
    }
    
    bool Symbol::operator== (const Symbol& d) const {
      return *_internal == *d._internal;
    } 
    
    Symbol::Symbol (const Symbol& pref, Symbol&& end) {
      if (end._internal->parent) {
	throw MiniMC::Support::Exception ("Cannot concatenate a non-root element to another symbol");	
      }

      end._internal->parent=pref._internal;
      _internal = std::move(end._internal);
    }
    
    Symbol::Symbol (const Symbol& s, std::string name) {
      _internal = std::make_shared<data> (name,s._internal);
    }

    Symbol::Symbol (std::shared_ptr<data> data) : _internal(std::move(data)) {   
    }
    
    MiniMC::Hash::hash_t Symbol::hash () const {
      return _internal->hash ();
    }

    
    //assumtion hasPrefix() == true
    Symbol Symbol::prefix () const {
      return Symbol (_internal->parent);
    }

    bool Symbol::hasPrefix () const {
      return _internal->parent != nullptr;	
    }

    bool Symbol::isRoot () const {
      return !hasPrefix ();
    }
    
    Symbol::~Symbol () {}
    
    std::ostream& Symbol::output (std::ostream& os ) const {
      return _internal->output ( os);
    }
    
    
    Symbol Symbol::from_string (const  std::string& str) {
      std::stringstream stream {str};
      std::string inp;
      std::getline (stream,inp,data::delim);
      Symbol symbol{inp};
      while (stream.good ())  {
	std::getline (stream,inp,data::delim);
	symbol = Symbol {symbol,inp};
      }
      
      return symbol;
    }
    
    struct Frame::Internal {
      Internal (Symbol symb) : symb(symb) {}
      Internal (Symbol symb,std::shared_ptr<Internal> p) : symb(symb),parent(p) {}
      
      
      bool resolve (const std::string& s, Symbol& symb) {
	auto it = symbols.find(s);
	if (it != symbols.end ()) {
	  symb = it->second;
	  return true;
	}
	  else if (parent){
	  return parent->resolve(s,symb);
	}
	else {
	  return false;
	}
      }


      
      template<class S>
      bool resolveRecursive (const std::string& qname, Symbol& symb, S s) {
	std::stringstream stream {qname};
	std::string inp;
	
	do   {
	  std::getline (stream,inp,Symbol::data::delim);  
	  if (stream.good ()) {
	    if (s->frames.count(inp))
	      s = s->frames.at(inp);
	    else
	      return false;
	  }
	  else  {
	    if (s->symbols.count(inp)) {
	      symb = s->symbols.at(inp);
	      return true;
	    }
	    else
	      return false;
	    
	  }
	}while (stream.good ());
	
	return false;
		
      }
      
      bool qualifiedResolve (const std::string& s, Symbol& symb) {
	auto p = parent;
	while (p->parent != nullptr)
	  p = p->parent;
	return resolveRecursive (s,symb,p);
      }

      Symbol symb;
      std::shared_ptr<Internal> parent;
      std::unordered_map<std::string, Symbol> symbols;
      std::unordered_map<std::string, std::shared_ptr<Internal>> frames;      
    };
    
    Frame::Frame (const std::string& s) : _internal(std::make_shared<Internal> (Symbol{s})) {}
    Frame::~Frame () {}

    
    
    Frame Frame::open (const std::string& s) {
      std::shared_ptr<Internal> _frame{nullptr};
      auto it  = _internal->frames.find (s);
      if (it== _internal->frames.end ())
	throw MiniMC::Support::Exception ("Cannot find frame");

      _frame = it->second;
      
      return _frame;
    }

    Frame Frame::create (const std::string& s) {
      if (_internal->frames.count (s)) {
	throw MiniMC::Support::Exception ("Frame already exists");
      }
      auto _frame = std::make_shared<Internal>(Symbol{_internal->symb,s},_internal);
      _internal->frames.emplace (s,_frame);
      
      return _frame;
    }
    
    Frame Frame::close () {
      if (!_internal->parent)
	throw MiniMC::Support::Exception ("Cannot close root scope");
      return Frame (_internal->parent);
    }

    bool Frame::resolve (const std::string& s, Symbol& symb) {
      return _internal->resolve( s,symb);
    }

    bool Frame::resolveQualified (const std::string& s, Symbol& symb) {
      return _internal->qualifiedResolve ( s,symb);
    }
    
    
    Symbol Frame::makeSymbol (const std::string& s) {
      Symbol symb{_internal->symb,s};
      _internal->symbols.emplace (s,symb);
      return symb;
    }
    
    Symbol Frame::makeFresh (const std::string& first) {
      std::size_t fresh_counter{0};
      auto newName = [first,&fresh_counter]() {
	std::stringstream str;
	str << "__minimc." << first << "_" <<++ fresh_counter;
	return str.str();
      };
      
      while(true) {
	auto freshname = newName ();
	if (!_internal->symbols.count(freshname))
	  return makeSymbol (freshname);
      }

    }
    
    
  }
}

