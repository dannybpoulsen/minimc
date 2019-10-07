#include "model/variables.hpp"

namespace MiniMC {
  namespace Model {
    Variable_ptr VariableStackDescr::addVariable (const std::string& name, const Type_ptr& type) {
      variables.push_back (std::make_shared<Variable> (name));
      variables.back()->setType(type);
      variables.back()->setOwnerId (id);
      variables.back()->setPlace ( totalSize);
      totalSize += type->getSize();
      return variables.back ();
    }
  }
}
