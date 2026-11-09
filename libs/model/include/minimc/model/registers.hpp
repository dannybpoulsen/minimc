#ifndef _REGISTERS__
#define _REGISTERS__

#include "minimc/util/valuemap.hpp"
#include "minimc/model/types.hpp"
#include "minimc/model/symbol.hpp"

namespace MiniMC {
  namespace Model {
    class RegisterInfo {
    public:
      explicit RegisterInfo(std::size_t id,Type_ptr type,RegType rtype = RegType::Local) :  id(id),reg_type(rtype),type(type) {}
      std::size_t getId() const { return id; }
      auto getRegType () const {return reg_type;}
      auto getType () const {return type;}
      
    private:
      std::size_t id;
      RegType reg_type;
      Type_ptr type;
    };

    class Register : public std::enable_shared_from_this<Register> {
    public:
      Register(const MiniMC::Model::Symbol::WeakSymbol symb, RegisterInfo&& p) : place(std::move(p)),symb(symb) {}
      
      auto getId  () const {return place.getId ();}
      auto getRegType() const { return place.getRegType(); }
      //In lack of better name
      auto getSystemType () const {return place.getType ();}
      auto getType () const {return place.getType ();}

      std::ostream& output(std::ostream& os) const {
        return os << "<" << getSymbol().getName() << " " << *getSystemType ()  << ">";
      }

      MiniMC::IO::ostream& output(MiniMC::IO::ostream& os) const {
        return os << "<" << getSymbol().getName() << " " << *getSystemType ()  << ">";
      }

      MiniMC::Model::Symbol getSymbol () const {return symb.getSymbol();}
      
    private:
      RegisterInfo place;
      MiniMC::Model::Symbol::WeakSymbol symb;
    };

    using Register_ptr = std::shared_ptr<Register>;

    
    inline std::ostream& operator<<(std::ostream& os, const Register& reg) {
      return reg.output(os);
    }

    class Value;
    using Value_ptr = std::shared_ptr<Value>;
    
    
    Value_ptr makeRegisterValue(const Register& reg);
    const Register& getRegister(const Value& value);
    Register_ptr getRegisterPtr(const Value& value);
        
    struct VariablePtrIndexer {
      
      std::size_t operator()(const Register& t) { return t.getId(); }
    };

    template <class T>
    using VariableMap = MiniMC::Util::FixedVector<Register, T, VariablePtrIndexer>;
    
    
  }
}

#endif
