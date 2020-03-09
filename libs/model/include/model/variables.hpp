#ifndef _VARIABLE__
#define _VARIABLE__

#include <string>
#include <limits>
#include <memory>
#include <vector>
#include <ostream>
#include "model/types.hpp"
#include "support/types.hpp"
namespace MiniMC {
  namespace Model {

    class Value {
    public:
      virtual ~Value () {}
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
      virtual ~Constant () {}
      bool isConstant () const {return true;}
      virtual const MiniMC::uint8_t* getData () const = 0; 
      virtual bool isAggregate () const {return false;}
  };

    class ConstantFactory64;
    
    class IntegerConstant :public Constant  {
    protected:
      IntegerConstant (uint64_t val) : value(val) {}
    public:
      friend class ConstantFactory64;
      
      auto& getValue () const {return value;}
      virtual const MiniMC::uint8_t* getData () const {return reinterpret_cast<const MiniMC::uint8_t*> (&value);}
      virtual std::ostream& output (std::ostream& os) const {
	os << "< " << value << " ";
	if (getType ())
	  os << *getType();
	else
	  os << "??";
	return os << " >";
      }
    private:
      uint64_t value;
    };

    class BinaryBlobConstant :public Constant  {
      protected:
      BinaryBlobConstant (MiniMC::uint8_t* data, std::size_t s) : value(new MiniMC::uint8_t[s]),size(s) {
	std::copy(data,data+s,value.get());
      }
    public:
      friend class ConstantFactory64;

      template<class T>
      auto& getValue () const {
	assert(sizeof(T) == size);
	return *reinterpret_cast<T*> (value.get());;
      }

      virtual const MiniMC::uint8_t* getData () const {return value.get();}
  
      virtual std::ostream& output (std::ostream& os) const {
	os << "< " << "BINARY(" << " ";
	for (size_t i = 0; i < size; i++) {
	  os <<  static_cast<int> (*(value.get()+i)) <<", ";
	}
	os << ") ";
	if (getType ())
	  os << *getType();
	else
	  os << "??";
	return os << " >";
      }
    private:
      std::unique_ptr<MiniMC::uint8_t[]> value;
      std::size_t size;
      };

    /*class AggregateConstant :public Constant  {
    public:
      friend class ConstantFactory64;
      
    protected:
      AggregateConstant (const std::vector<Value_ptr>& vals, bool isarr) : values(vals),
									   is_Array(isarr)
      {
      }
    public:
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
      };*/

    
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
      virtual std::ostream& output (std::ostream& os) const  {
		os << " < " << getName() << " ";
		if (getType()) 
		  os  << *getType() ;
		else {
		  os << " ___ ";
		}
		return os << " >";
	  }
      bool isVariable () const {return true;}
      void setOwner (const VariableStackDescr_ptr& descr) {owner = descr.get();}
      auto& getOwner () const  {return owner;}
      
    private:
      std::string name;
      VariableStackDescr* owner;
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


    class ConstantFactory {
    public:
      ConstantFactory () {}
      virtual ~ConstantFactory () {}
      
      using aggr_input = std::vector<Value_ptr>;
      virtual const Value_ptr makeAggregateConstant (const aggr_input& inp,bool) = 0;
      virtual const Value_ptr makeIntegerConstant (MiniMC::uint64_t) = 0;
      virtual const Value_ptr makeBinaryBlobConstant (MiniMC::uint8_t*, std::size_t) = 0;
      
      virtual const Value_ptr makeLocationPointer (MiniMC::func_t,MiniMC::offset_t) = 0;
      virtual const Value_ptr makeFunctionPointer (MiniMC::func_t) = 0;
    };

    class ConstantFactory64 : public ConstantFactory {
    public:
      
      ConstantFactory64 () {}
      virtual ~ConstantFactory64 () {}
      virtual const Value_ptr makeIntegerConstant (MiniMC::uint64_t);
      virtual const Value_ptr makeBinaryBlobConstant (MiniMC::uint8_t*, std::size_t);
      virtual const Value_ptr makeAggregateConstant (const aggr_input& inp,bool);
      virtual const Value_ptr makeLocationPointer (MiniMC::func_t,MiniMC::offset_t);
      virtual const Value_ptr makeFunctionPointer (MiniMC::func_t);
      
    };

    using ConstantFactory_ptr = std::shared_ptr<ConstantFactory>;
    
  }
}

#endif
