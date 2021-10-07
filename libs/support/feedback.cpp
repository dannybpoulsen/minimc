#include "support/feedback.hpp"

namespace MiniMC {
  namespace Support {

    Messager_ptr makeMessager(MessagerType);
    Messager_ptr act_messager = std::make_unique<Messager>();

    void setMessager(MessagerType term) {
      act_messager = makeMessager(term);
    }

    Messager& getMessager() { return *act_messager; }

  } // namespace Support
} // namespace MiniMC
