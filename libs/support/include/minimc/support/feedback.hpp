#ifndef _FEEDBACK__
#define _FEEDBACK__


#include <memory>
#include <sstream>
#include <type_traits>

#include <future>
using namespace std::chrono_literals;
using namespace std::chrono_literals;


namespace MiniMC {
  namespace Support {

    enum class Severity {
      Error,
      Warning,
      Info,
      Progress,
      Submessage
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
    using SubMessage = Message<Severity::Submessage>;
    
    
    
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

    template<class T>
    using TSubmessage = TMessage<T,Severity::Submessage>;
    
    
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
      virtual void mess(const SubMessage&) {}
      
      static std::shared_ptr<MessageSink> make (MessageSinkType);
      static std::shared_ptr<MessageSink> defaultSink ();
      
    };
    
    
    
    
    class Messager {
    public:
      Messager (std::shared_ptr<MessageSink> sink = MessageSink::defaultSink ()) : sink(std::move(sink)) {}
      
      
      template<class T>
      auto& operator<< (T&& mess) {sink->mess(mess); return *this;}
      
      
    private:
      std::shared_ptr<MessageSink> sink; 
      
    };

    class AsyncExecutor {
    public: 
      AsyncExecutor (Messager messager) : messager(std::move(messager)) {}
    
      template<class ProgressMessage,class Func,class... Args>
      auto execute (ProgressMessage mess, Func f, Args... args) {
	messager << mess;
	auto promise = std::async (f,args...);
	std::future_status status;
	status = promise.wait_for(500ms);
	
	while (status!=std::future_status::ready) {
	  messager << mess;
	  status = promise.wait_for(500ms);
	  
	};
	return promise.get();
      }
    private:
      Messager messager;
    };
    
  } // namespace Support
} // namespace MiniMC

#endif
