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
    
    
    class MessageSink {
    public:
      virtual ~MessageSink() {}
      /*virtual void error(const std::string&) {}
      virtual void warning(const std::string&) {}
      virtual void message(const std::string&) {}
      virtual void progress(const std::string&) {}
      */
      virtual void mess(const ErrorMessage&) {}
      virtual void mess(const WarningMessage&) {}
      virtual void mess(const InfoMessage&) {}
      virtual void mess(const ProgressMessage&) {}
      
    };
    
    enum class MessagerType {
      Terminal
    };
    
    
    class Messager {
    public:
      template<Severity severity = Severity::Info>
      void message (const std::string& s) {
	sink->mess (TMessage<std::string,severity> {s});
      }
      
      template<Severity s = Severity::Info, class T>   requires (!std::is_same_v<T,std::string>)
      void message (const T& t) {
	std::stringstream str;
	str << t;
	message<s> ( str.str ());
      }
      
      static void setMessageSink(MessagerType);
      
      
    private:
      static std::unique_ptr<MessageSink> sink; 
      
    };
    
    
    
  } // namespace Support
} // namespace MiniMC

#endif
