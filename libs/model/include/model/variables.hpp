#ifndef _VARIABLE__
#define _VARIABLE__

#include <string>
#include <limits>
#include <memory>
#include <vector>
#include <ostream>
#include <sstream>
#include <algorithm>
#include "model/types.hpp"
#include "support/exceptions.hpp"
#include "support/storehelp.hpp"
#include "support/types.hpp"
#include "support/binary_encode.hpp"
#include "util/valuemap.hpp"

namespace MiniMC {
  namespace Model {

	/**
	 * 
	 * Abstract representation of values in MiniMC. 
	 * A Value is associated to Type, and be either Variables or Constants.
	 * Values can also be local or global to a given function. 
	 */
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
      const std::string string_repr () const {
	std::stringstream str;
	this->output (str);
	return str.str();
      }

      operator std::string () const {
	return this->string_repr ();
      }
      
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
      bool isConstant () const override {return true;}  
      virtual const MiniMC::uint8_t* getData () const = 0;
      virtual std::size_t getSize () {return 0;}
      virtual bool isAggregate () const {return false;}
      virtual bool isInteger () const {return false;}
      virtual bool isBinaryBlobConstant () const {return false;}
      virtual bool isNonCompileConstant () const {return false;}
    };
    
    class NonCompileConstant : public Constant {
    public:
      virtual ~NonCompileConstant () {}
      bool isNonCompileConstant () const {return true;}  
      virtual bool isAggregate () const {return true;}
	};

    class AggregateNonCompileConstant : public NonCompileConstant {
    public:
      AggregateNonCompileConstant (const std::vector<Value_ptr>& val) : ops(val) {}
      auto begin () const {return ops.begin();}
      auto end () const {return ops.end();}
      
      virtual std::ostream& output (std::ostream& os) const  {
	os << "[";
	std::for_each (ops.begin(),ops.end(),[&](const Value_ptr& v){os << *v << ",";});
	return os << "]";
      }
      
      virtual const MiniMC::uint8_t* getData () const {
	throw MiniMC::Support::Exception ("GetData should not be called on NonCompileConstants");
      }
      
    private:
      std::vector<Value_ptr> ops;
    };
    
    using NonCompileConstant_ptr = std::shared_ptr<NonCompileConstant>;
    using Constant_ptr = std::shared_ptr<Constant>;
    
    class ConstantFactory64;

    template<typename T>
    class IntegerConstant :public Constant  {
    protected:
      IntegerConstant (T val) : value(0) {
		MiniMC::saveHelper<T>(reinterpret_cast<MiniMC::uint8_t*>(&value),sizeof(value)) = val;
      }
    public:
      friend class ConstantFactory64;
      
      T getValue () const {
	auto val =  MiniMC::loadHelper<T>(reinterpret_cast<const MiniMC::uint8_t*>(&value),sizeof(value));
	return val;
      }

      virtual std::size_t getSize () override {return sizeof(T);}
      
	  
      virtual const MiniMC::uint8_t* getData () const {
	return reinterpret_cast<const MiniMC::uint8_t*> (&value);
      }
      
      virtual bool isInteger () const {return true;}
      
      virtual std::ostream& output (std::ostream& os) const {
	MiniMC::Support::Base64Encode encoder;
	os << encoder.encode (reinterpret_cast<const char*> (&value),sizeof(T));
	//os << value << std::endl;
	if (getType ())
	  os << *getType();
	else
	  os << "??";
	return os << " >";
      }
    private:
      T value;
    };

	/**
	 * Class for representing binary blobs which are useful when having to represent constant arrays/structs.
	 *
	 */ 
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
      
      virtual const MiniMC::uint8_t* getData () const {
		return value.get();
      }

      virtual bool isBinaryBlobConstant () const {return true;}
      
      std::size_t getSize () override {return size;}
      
      virtual std::ostream& output (std::ostream& os) const {
	MiniMC::Support::Base64Encode encoder;
	os << encoder.encode (reinterpret_cast<const char*> (value.get()),size);
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


	/**
	 * Representation of Variable in MiniMC. 
	 * A variable is associated to an owning VariableStackDescr that sets its id and byte-placement in an activation record during execution  
	 */
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

	/**
	 * VariableStackDescr describes the structure of an activation record (in respect to variables in MiniMC - not 
	 * stack allocations). 
	 *
	 */
    class VariableStackDescr : public std::enable_shared_from_this<VariableStackDescr> {
    public:
      VariableStackDescr (const std::string& pref) : pref(pref) {}
      Variable_ptr addVariable (const std::string& name, const Type_ptr& type); 
      auto& getVariables () const {return variables;}

      /** 
       *
       * @return Total size in bytes of an activation record
       */
      auto getTotalSize () const {return totalSize;}
      auto getTotalVariables () const {return variables.size();}
    private:
      std::vector<Variable_ptr> variables;
      std::size_t totalSize = 0;
      const std::string pref;
    };


    class ConstantFactory {
    public:
      ConstantFactory () {}
      virtual ~ConstantFactory () {}
      
      using aggr_input = std::vector<Constant_ptr>;
      using noncompile_aggr_input = std::vector<Value_ptr>;
      virtual const Value_ptr makeAggregateConstant (const aggr_input& inp,bool) = 0;
      virtual const Value_ptr makeAggregateConstantNonCompile (const noncompile_aggr_input& inp,bool) = 0;
      virtual const Value_ptr makeIntegerConstant (MiniMC::uint64_t, const Type_ptr&) = 0;
      virtual const Value_ptr makeBinaryBlobConstant (MiniMC::uint8_t*, std::size_t) = 0;
      virtual const Value_ptr makeLocationPointer (MiniMC::func_t,MiniMC::offset_t) = 0;
      virtual const Value_ptr makeFunctionPointer (MiniMC::func_t) = 0;
    };

    class ConstantFactory64 : public ConstantFactory {
    public:
      
      ConstantFactory64 () {}
      virtual ~ConstantFactory64 () {}
      virtual const Value_ptr makeIntegerConstant (MiniMC::uint64_t, const Type_ptr&);
      virtual const Value_ptr makeBinaryBlobConstant (MiniMC::uint8_t*, std::size_t);
      virtual const Value_ptr makeAggregateConstantNonCompile (const noncompile_aggr_input& inp,bool);
      virtual const Value_ptr makeAggregateConstant (const aggr_input& inp,bool);
      virtual const Value_ptr makeLocationPointer (MiniMC::func_t,MiniMC::offset_t);
      virtual const Value_ptr makeFunctionPointer (MiniMC::func_t);
      
    };
    using ConstantFactory_ptr = std::shared_ptr<ConstantFactory>;

    struct VariablePtrIndexer {
      std::size_t operator() (const Variable_ptr& t) {return t->getId ();}
      
    };
    
    template<class T>
    using VariableMap = MiniMC::Util::FixedVector<Variable_ptr,T,VariablePtrIndexer>; 
    
  }
}




 
#endif
