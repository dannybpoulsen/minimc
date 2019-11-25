#ifndef _VARIABLE__
#define _VARIABLE__

#include <string>
#include <limits>
#include <memory>
#include <vector>
#include <ostream>
#include "model/types.hpp"
namespace MiniMC {
  namespace Model {

    class Value {
    public:
      ~Value () {}
      const Type_ptr& getType  () const  {return type;}
      void setType (const Type_ptr& t) {type = t;}
      virtual bool isVariable () const {return false;}
      virtual bool isConstant () const {return false;}
      virtual bool isGlobal () const {return glob;}
      virtual void setGlobal () {glob = true;}
      
      virtual std::ostream& output (std::ostream& os) const = 0;
    private:
      Type_ptr type;
      bool glob = false;
    };

    inline std::ostream& operator<< (std::ostream& os, const Value& v) {
      return v.output (os);
    }
    
    using Value_ptr = std::shared_ptr<Value>;

    class Constant : public Value {
    public:
      bool isConstant () const {return true;}
      virtual bool isAggregate () const {return false;}
  };
    
    class IntegerConstant :public Constant  {
	public:
      IntegerConstant (uint64_t val) : value(val) {}
      auto getValue () const {return value;}
  
      virtual std::ostream& output (std::ostream& os) const {return os << value;}
    private:
      uint64_t value;
    };

    class AggregateConstant :public Constant  {
    public:
      AggregateConstant (std::vector<Value_ptr>& vals, bool isarr) : values(vals),
								     is_Array(isarr)
      {}
      auto& getValues () const {return values;}
      bool isArray () const {return is_Array;}
      bool isAggregate () const {return true;}
  
      virtual std::ostream& output (std::ostream& os) const {
	const std::string start = is_Array ? "[ " : "{ ";
	const std::string stop = is_Array ? "]" : "}";
	os << start;
	for (auto& v : values)
	  os << *v << " " ;
	return os << stop;
      }
    private:
      std::vector<Value_ptr> values;
      bool is_Array = false;
    };

    
    template<class T>
    class Placed  {
    public:
      Placed () : place(unused),id(unused) {}
      std::size_t getPlace () const {return place;}
      std::size_t getId () const {return id;}
      
      void setPlace (std::size_t p) {place = p;}
      void setId (std::size_t i)  {id = i;} 
      
    private:
      static constexpr std::size_t unused = std::numeric_limits<std::size_t>::max ();
      std::size_t place;
      std::size_t id;
    };

    class VariableStackDescr;
    using VariableStackDescr_ptr = std::shared_ptr<VariableStackDescr>;
    
    
    class Variable : public Value,
		     public Placed<Variable>,
		     public std::enable_shared_from_this<Variable> 
    {
    public:
      Variable (const std::string& name) : name(name) {}
      const std::string& getName () const {return name;}
      virtual std::ostream& output (std::ostream& os) const  {return os << getName();}
      bool isVariable () const {return true;}
      void setOwner (const VariableStackDescr_ptr& descr) {owner = descr;}
      auto& getOwner () const  {return owner;}
      
    private:
      std::string name;
      Type_ptr type;
      VariableStackDescr_ptr owner;
    };

    using Variable_ptr = std::shared_ptr<Variable>;
    
    class VariableStackDescr : public std::enable_shared_from_this<VariableStackDescr> {
    public:
      VariableStackDescr ()  {}
      Variable_ptr addVariable (const std::string& name, const Type_ptr& type); 
      auto& getVariables () const {return variables;}
      auto getTotalSize () const {return totalSize;}
      auto getTotalVariables () const {return variables.size();}
    private:
      std::vector<Variable_ptr> variables;
      std::size_t totalSize = 0;
    
    };

    
  }
}

#endif
