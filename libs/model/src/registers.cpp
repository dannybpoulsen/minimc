
#include "minimc/model/registers.hpp"
#include "minimc/model/variables.hpp"


namespace MiniMC {
  namespace Model {
    Value_ptr makeRegisterValue(const Register_ptr& reg) {
      auto symbol = reg->getSymbol();
      auto value = SymbolicConstant::make(std::move(symbol));
      value->setType(reg->getSystemType());
      return value;
    }

    Register_ptr getRegister(const Value_ptr& value) {
      auto symbolic = std::dynamic_pointer_cast<SymbolicConstant>(value);
      if (symbolic) {
        auto symbol = symbolic->getValue();
        if (auto reg = std::get_if<Register_ptr>(&symbol.getUserData())) {
          return *reg;
        }
      }
      throw MiniMC::Support::Exception("Value does not refer to a register");
    }
    
    
  }
}
