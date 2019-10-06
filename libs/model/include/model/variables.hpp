#ifndef _VARIABLE__
#define _VARIABLE__

#include <string>
#include "model/types.hpp"
namespace MiniMC {
  namespace Model {

	template<class T>
	class IDed {
	public:
	  IDed (std::size_t id) : id(id) {}
	  std::size_t getID () const {return id;}
	private:
	  std::size_t id;
	};
	
	class Variable : public IDed<Variable> {
	public:
	  Variable (std::size_t id, const std::string& name) : name(name) {}
	  Type* getType const ()  {return type;}
	  void setType (Type* t) {type = t;}
	  const std::string& getName () const {return name;}
	private:
	  std::string name;
	  Type* type;
	};
  }
}

#endif
