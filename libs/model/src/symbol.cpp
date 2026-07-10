#include "minimc/model/symbol.hpp"
#include "minimc/support/exceptions.hpp"
#include "minimc/hash/hashing.hpp"
#include "minimc/support/overload.hpp"

#include <stdexcept>
#include <string>
#include <ostream>
#include <sstream>
#include <iostream>
#include <memory>
#include <list>
#include <cassert>
#include <algorithm>
#include <variant>

namespace MiniMC {
  namespace Model {
    struct Symbol::data {
      static const char delim {':'};
      data (std::shared_ptr<data> p = nullptr) : parent(std::move(p)) {}
      data (std::string s,  std::shared_ptr<data> p = nullptr) : parent(std::move(p)),name(s) {}
      
      std::ostream& output (std::ostream& os) const {
	if (parent ) {
	  return parent->output (os) << delim << name;
	  
	}
	else
	  return os << name;
      }

      MiniMC::IO::ostream& output (MiniMC::IO::ostream& os) const {
	if (parent ) {
	  return parent->output (os) << delim << name;
	  
	}
	else
	  return os << name;
      }
      
      bool isRoot () const {
	return parent == nullptr;
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
      UserData userdata{std::monostate{}};
    };

    
    const UserData& Symbol::getUserData  () const {return _internal->userdata;}
    void Symbol::setUserData (UserData userdata)  {_internal->userdata = userdata;}
    
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
    
    MiniMC::IO::ostream& Symbol::output (MiniMC::IO::ostream& os ) const {
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
    
    struct Frame::Internal : public std::enable_shared_from_this<Internal> {
      Internal (Symbol symb) : symb(symb) {}
      Internal (Symbol symb,std::weak_ptr<Internal> p) : symb(symb),parent(p) {}
      
      
      std::optional<Symbol>resolve (const std::string& s) const {
	auto it = symbols.find(s);
	if (it != symbols.end ()) {
	  return it->second;
	  
	}
	else if (!parent.expired ()){
	  return parent.lock ()->resolve(s);
	}
	else {
	  return std::nullopt;
	}
      }


      
      template<class S>
      std::optional<Symbol> resolveRecursive (const std::string& qname, S s) const {
	std::stringstream stream {qname};
	std::string inp;

	if (this->symb.isRoot () && this->symb.getFullName () != "") {
	  std::getline (stream,inp,Symbol::data::delim);
	  if(inp != this->symb.getName())
	    return std::nullopt;
	}
	
	do {
	  std::getline (stream,inp,Symbol::data::delim);
	  if (stream.good ()) {
	    if (s->frames.count(inp))
	      s = s->frames.at(inp);
	    else
	      return std::nullopt;;
	  }
	  else  {
	    if (s->symbols.count(inp)) {
	      return s->symbols.at(inp);
	      
	    }
	    else
	      return std::nullopt;
	    
	  }
	}while (stream.good ());
	
	return std::nullopt;
		
      }
      
      std::optional<Symbol> qualifiedResolve (const std::string& s)const {
	if (parent.expired ())
	  return resolveRecursive (s,this->shared_from_this());
	auto p = parent;
	while (!p.expired ()) {
	  auto q = p.lock ();
	  if (q->parent.expired ())
	    return resolveRecursive (s,q);
	  else
	    p = q->parent;
	}
	return std::nullopt;
      }

      std::generator<Symbol> gen_symbols () {
	for (auto& s : symbols) {
	  co_yield s.second;
	}

	for (auto& f : frames) {
	  co_yield std::ranges::elements_of (f.second->gen_symbols());
	}
	
      }
      
      std::generator<Symbol> gen_parent_symbols () {
	for (auto& s : symbols) {
	  co_yield s.second;
	}

	auto p = parent.lock();
	if (p) {
	  co_yield std::ranges::elements_of (p->gen_parent_symbols());
	  
	}
	
      }


      Symbol symb;
      std::weak_ptr<Internal> parent;
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
      if (_internal->parent.expired ())
	throw MiniMC::Support::Exception ("Cannot close root scope");
      return Frame (_internal->parent.lock ());
    }

    std::optional<Symbol> Frame::resolve (const std::string& s) const{
      return _internal->resolve( s);
    }

    std::optional<Symbol> Frame::resolveQualified (const std::string& s) const {
      return _internal->qualifiedResolve ( s);
    }
    
    
    Symbol Frame::makeSymbol (const std::string& s) {
      Symbol symb{_internal->symb,s};
      if (_internal->symbols.count (s))
	throw MiniMC::Support::Exception (MiniMC::Support::Localiser ("Symbol '%1%' already defined").format (symb));
      _internal->symbols.emplace (s,symb);
      return symb;
    }
    
    Symbol Frame::makeFresh (const std::string& first) {
      std::size_t fresh_counter{0};
      auto newName = [first,&fresh_counter]() {
	std::stringstream str;
	str << first;
	if (fresh_counter != 0)
	  str << "_" << fresh_counter;
	fresh_counter++;
	return str.str();
      };
      
      while(true) {
	auto freshname = newName ();
	if (!_internal->symbols.count(freshname))
	  return makeSymbol (freshname);
      }

    }
    
    std::generator<Symbol> Frame::symbols () const {
      co_yield std::ranges::elements_of(_internal->gen_symbols());
    }
    
    std::generator<Symbol> Frame::local_symbols () const {
      for (auto& s : _internal->symbols) {
	co_yield s.second;
      }
    }

    std::generator<Symbol> Frame::local_and_parent_symbols () const {
      co_yield std::ranges::elements_of(_internal->gen_parent_symbols());
    }

    std::generator<std::tuple<Symbol,MiniMC::Model::Register_ptr>> Frame::local_registers () const {
      for (auto t : local_symbols()) {
	if (std::holds_alternative<MiniMC::Model::Register_wptr> (t.getUserData()))
	  co_yield std::make_tuple(t,std::get<MiniMC::Model::Register_wptr> (t.getUserData()).lock());
      }
    }

    std::size_t Frame::numberOfRegisters () const {
      std::size_t regs{0};
      for (auto t : local_symbols()) {
	if (std::holds_alternative<MiniMC::Model::Register_wptr> (t.getUserData()))
	  regs++;
      }
      return regs;
    }
    
    bool Frame::hasSymbol (const Symbol& s) const {
      for (auto ss : local_symbols()) {
	if (s == ss)
	  return true;
      }
      return false;
    }
    
  }
}

