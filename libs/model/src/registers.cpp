
#include "minimc/model/registers.hpp"
#include "minimc/model/variables.hpp"


namespace MiniMC {
  namespace Model {
    namespace {
      const Register_ptr& getRegisterHandle(const Value& value) {
        auto symbolic = dynamic_cast<const SymbolicConstant*>(&value);
        if (symbolic) {
          auto symbol = symbolic->getValue();
          if (auto reg = std::get_if<Register_ptr>(&symbol.getUserData())) {
            return *reg;
          }
        }
        throw MiniMC::Support::Exception("Value does not refer to a register");
      }
    }

    Value_ptr makeRegisterValue(const Register& reg) {
      auto symbol = reg.getSymbol();
      auto value = SymbolicConstant::make(std::move(symbol));
      value->setType(reg.getSystemType());
      return value;
    }

    const Register& getRegister(const Value& value) {
      return *getRegisterHandle(value);
    }

    Register_ptr getRegisterPtr(const Value& value) {
      return getRegisterHandle(value);
    }

    Register_ptr RegisterFactory::makeRegister(MiniMC::Model::Symbol symbol, const Type_ptr& type) {
      auto reg = std::make_shared<Register>(symbol.weak(),
					    RegisterInfo{_internal->variable_map.size(), type, _internal->types});
      _internal->variable_map[symbol.getName()] = reg;
      symbol.setUserData (reg);
      
      
      return reg;
    }
    
    
  }
}
