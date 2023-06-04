#include "support/feedback.hpp"

namespace MiniMC {
  namespace Support {

    std::shared_ptr<MessageSink> makeMessager(MessageSinkType);
    
    
    std::shared_ptr<MessageSink> MessageSink::make(MessageSinkType term) {
      return makeMessager(term);
    }

    std::shared_ptr<MessageSink> defSink = makeMessager (MessageSinkType::Terminal);
    
    std::shared_ptr<MessageSink> MessageSink::defaultSink () {
      return defSink;
    }
    
  } // namespace Support
} // namespace MiniMC
