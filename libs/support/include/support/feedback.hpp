#ifndef _FEEDBACK__
#define _FEEDBACK__


#include <memory>
#include <sstream>
#include <type_traits>

namespace MiniMC {
  namespace Support {

    enum class Severity {
      Error,
      Warning,
      Info,
      Progress
    };
    
    template<Severity type>
    class Message {
    public:
      Message ()  {} 
      virtual ~Message () {}
      virtual std::ostream& to_string (std::ostream& ) const = 0;
      auto getType () const {return type;}
    };

    using ErrorMessage = Message<Severity::Error>;
    using WarningMessage = Message<Severity::Warning>;
    using InfoMessage = Message<Severity::Info>;
    using ProgressMessage = Message<Severity::Progress>;
    
    
    
    template<class T, Severity t>
    class TMessage : public Message<t> {
    public:
      TMessage (T m) : item(std::move(m)) {}
      virtual std::ostream& to_string (std::ostream& os) const {
	return os << item;
      }
          private:
      T item;
    };

    template<class T>
    using TError = TMessage<T,Severity::Error>;

    template<class T>
    using TWarning = TMessage<T,Severity::Warning>;

    template<class T>
    using TInfo = TMessage<T,Severity::Info>;

    template<class T>
    using TProgress = TMessage<T,Severity::Progress>;
    
    
    enum class MessageSinkType {
      Terminal
    };
    
    
    class MessageSink {
    public:
      virtual ~MessageSink() {}
      virtual void mess(const ErrorMessage&) {}
      virtual void mess(const WarningMessage&) {}
      virtual void mess(const InfoMessage&) {}
      virtual void mess(const ProgressMessage&) {}
      
      static std::shared_ptr<MessageSink> make (MessageSinkType);
      
    };
    
    
    
    
    class Messager {
    public:
      Messager (std::shared_ptr<MessageSink>&& sink = MessageSink::make (MessageSinkType::Terminal)) : sink(std::move(sink)) {}



      template<class T>
      auto& operator<< (T&& mess) {sink->mess(mess); return *this;}
      
      
    private:
      std::shared_ptr<MessageSink> sink; 
      
    };
    
    
    
  } // namespace Support
} // namespace MiniMC

#endif
