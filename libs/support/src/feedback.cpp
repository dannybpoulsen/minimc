#include "minimc/support/feedback.hpp"

namespace MiniMC {
  namespace Support {
    std::shared_ptr<MessageSink> defSink = std::make_shared<MessagePipeline> ();
    
    void MessageSink::setDefaultSink (std::shared_ptr<MessageSink> d) {
      defSink = d;
    }
    
    std::shared_ptr<MessageSink> MessageSink::defaultSink () {
      return defSink;
    }
    
  } // namespace Support
} // namespace MiniMC
