#include "support/feedback.hpp"

namespace MiniMC {
  namespace Support {

    std::unique_ptr<MessageSink> makeMessager(MessagerType);
    std::unique_ptr<MessageSink> Messager::sink = makeMessager(MessagerType::Terminal); 
    
    void Messager::setMessageSink(MessagerType term) {
      sink = makeMessager(term);
    }

  } // namespace Support
} // namespace MiniMC
