#include "support/feedback.hpp"

namespace MiniMC {
  namespace Support {

    std::unique_ptr<MessageSink> makeMessager(MessagerType);
    std::unique_ptr<MessageSink> act_messager = std::make_unique<MessageSink>();
    
    void setMessageSink(MessagerType term) {
      act_messager = makeMessager(term);
    }

    MessageSink& getMessager() { return *act_messager; }

  } // namespace Support
} // namespace MiniMC
