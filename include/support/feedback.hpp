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
    
    
    class MessageSink {
    public:
      virtual ~MessageSink() {}
      virtual void error(const std::string&) {}
      virtual void warning(const std::string&) {}
      virtual void message(const std::string&) {}
      virtual void progress(const std::string&) {}
    
    };

    
    
    MessageSink& getMessager();
    
    
    class Messager {
    public:
      template<Severity severity = Severity::Info>
      void message (const std::string& s) {
	if constexpr (severity == Severity::Info) {
	  getMessager ().message (s);
	}
	else if constexpr (severity == Severity::Warning) {
	  getMessager ().warning (s);
	}
	else if constexpr (severity == Severity::Error) {
	  getMessager ().error (s);
	}

	else if constexpr (severity == Severity::Progress) {
	  getMessager ().progress (s);
	}
	
      }

      template<Severity s = Severity::Info, class T>   requires (!std::is_same_v<T,std::string>)
      void message (const T& t) {
	std::stringstream str;
	str << t;
	message<s> ( str.str ());
      }
	
    };
    
    enum class MessagerType {
      Terminal
    };
    
    void setMessageSink(MessagerType);
    
  } // namespace Support
} // namespace MiniMC

#endif
