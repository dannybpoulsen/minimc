#include "support/feedback.hpp"

namespace MiniMC {
  namespace Support {

    std::shared_ptr<MessageSink> makeMessager(MessageSinkType);
    
    
    std::shared_ptr<MessageSink> MessageSink::make(MessageSinkType term) {
      return makeMessager(term);
    }

  } // namespace Support
} // namespace MiniMC
