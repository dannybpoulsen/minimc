#include "minimc/support/feedback.hpp"

namespace MiniMC {
  namespace Support {
    std::shared_ptr<MessageSink> defSink = std::make_shared<MessagePipeline> ();
    
    void MessageSink::setDefaultSink (std::shared_ptr<MessageSink>&& d) {
      std::swap(defSink,d);
    }
    
    std::shared_ptr<MessageSink> MessageSink::defaultSink () {
      return defSink;
    }

    std::shared_ptr<InteractionSource> defSource = std::make_shared<InteractionSource> ();
    
    void InteractionSource::setDefaultSource (std::shared_ptr<InteractionSource>&& d) {
      std::swap(defSource,d);
    }
    
    std::shared_ptr<InteractionSource> InteractionSource::defaultSource () {
      return defSource;
    }
    
  } // namespace Support
} // namespace MiniMC
