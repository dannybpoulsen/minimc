#include "minimc/support/feedback.hpp"

namespace MiniMC {
  namespace Support {
    std::unique_ptr<MessageSink> defSink = std::make_unique<MessagePipeline> ();
    
    void MessageSink::setDefaultSink (std::unique_ptr<MessageSink>&& d) {
      std::swap(defSink,d);
    }
    
    std::unique_ptr<MessageSink>& MessageSink::defaultSink () {
      return defSink;
    }
    
  } // namespace Support
} // namespace MiniMC
