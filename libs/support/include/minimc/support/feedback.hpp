#ifndef _FEEDBACK__
#define _FEEDBACK__


#include <memory>
#include <sstream>
#include <type_traits>

#include <future>
#include <list>
#include <optional>
#include "minimc/io/ostream.hpp"


using namespace std::chrono_literals;
using namespace std::chrono_literals;


namespace MiniMC {
  namespace Support {

    enum class Severity {
      Error,
      Warning,
      Info,
      Progress,
      SubProgress
    };

    
    
    class Message {
    public:
      Message ()  {} 
      virtual ~Message () {}
      virtual MiniMC::IO::ostream& to_string (MiniMC::IO::ostream& ) const = 0;
      
      virtual Severity getType () const = 0;
    };

    template<class M>
    concept HasToString = requires (const M& m, MiniMC::IO::ostream& os) {
      {m.to_string (os)};
    };
    
    
    template<HasToString M>
    inline MiniMC::IO::ostream& operator<< (MiniMC::IO::ostream& os, const M& m) {
      return m.to_string(os); 
    }
    
    
     template<Severity t>
    class MessageT : public Message {
    public:
      Severity getType() const override {return t;}
       
    };
    

    using ErrorMessage = MessageT<Severity::Error>;
    using WarningMessage = MessageT<Severity::Warning>;
    using InfoMessage = MessageT<Severity::Info>;
    using ProgressMessage = MessageT<Severity::Progress>;
    using SubProgressMessage = MessageT<Severity::SubProgress>;
    
    template<class M>
    concept Outputtable = requires (const M& m, MiniMC::IO::ostream& os) {
      {os << m};
    };
    
    template<Outputtable T, Severity t>
    class TMessage : public MessageT<t> {
    public:
      TMessage (T m) : item(m) {}
      virtual MiniMC::IO::ostream& to_string (MiniMC::IO::ostream& os) const {
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
    using TSubProgress = TMessage<T,Severity::SubProgress>;
    
    
    
    
    class MessageSink {
    public:
      virtual ~MessageSink() {}
      virtual void mess(const Message&) {}
      virtual void pumpProgress() {}
      virtual MiniMC::IO::ostream& raw_stream (Severity) = 0;
      
      static std::unique_ptr<MessageSink>& defaultSink ();
      static void setDefaultSink (std::unique_ptr<MessageSink>&&);
      
    };
    
    class MessageHandler {
    public:
      virtual bool handle (const Message& ) = 0;
      virtual void pump () = 0;
      virtual std::optional<MiniMC::IO::ostream*> raw_stream (Severity)  = 0;
    };

    class MessagePipeline : public MessageSink {
    public:
      MessagePipeline ()  {}
      
      MessagePipeline (std::list<std::unique_ptr<MessageHandler>>&& handlers) : handlers(std::move(handlers))  {}
      void mess(const Message& m) override {
	for (auto& s : handlers) {
	  if (s->handle(m))
	    break;
	}
      }

      void pumpProgress() override {
	std::for_each (handlers.begin(),handlers.end(),[](auto& g) {g->pump();});
      }

      virtual MiniMC::IO::ostream& raw_stream (Severity sev) override {
	for (auto& s : handlers) {
	  {
	    if (auto stream = s->raw_stream ( sev))
	      return *stream.value();
	    
	  }
	}

	return MiniMC::IO::os_ostream::err();
	
      }
      
      
      
    private:
      std::list<std::unique_ptr<MessageHandler>> handlers;
    };

    class MessagePipelineBuilder {
    public:
      template<class T,class... Args>
      auto add (Args&&... args) {
	handlers.push_back (std::make_unique<T>(std::forward<Args>(args)...));
      }

      std::unique_ptr<MessagePipeline> build() {
	return std::make_unique<MessagePipeline> (std::move(handlers));
      }
      
    private:
      std::list<std::unique_ptr<MessageHandler>> handlers;
      
    };

    template<Severity sev>
    class StreamHandler : public MessageHandler {
    public:
      StreamHandler (MiniMC::IO::ostream& o) : stream(o) {}
      virtual void pump () override;
      virtual bool handle (const Message&) override;
      std::optional<MiniMC::IO::ostream*> raw_stream (Severity s) override {
	if (sev == s)
	  return {&stream};
	return std::nullopt;
      }
      
    protected:
      MiniMC::IO::ostream& stream;
    };

    class ProgressStreamHandler : public MessageHandler {
    public:
      ProgressStreamHandler (MiniMC::IO::ostream& o) : stream(o) {}
      bool handle (const Message&) override;
      void pump () override;
      virtual std::optional<MiniMC::IO::ostream*> raw_stream (Severity s) {
	if (s== Severity::Progress || s== Severity::SubProgress)
	  return &stream;
	else
	  return std::nullopt;
      };
    
    private:
      MiniMC::IO::str_ostream buffer;
      MiniMC::IO::str_ostream sub_message;
      MiniMC::IO::ostream& stream;
    
    };
    
    
    
    class Messager {
    public:
      Messager () {}
      
      
      template<class T>
      auto& operator<< (T&& mess) requires std::derived_from<T,Message> {
	MessageSink::defaultSink()->mess(mess);
	return *this;
      }
      
      MiniMC::IO::ostream& raw_stream (Severity sev) {
	return MessageSink::defaultSink()->raw_stream(sev);
      }
      
      void pumpProgress () {MessageSink::defaultSink()->pumpProgress();}
      
    private:
      
    };

    template<class T>
    class SubProgresSenderClearer {
    public:
      SubProgresSenderClearer (const T& mess) {
	Messager{} << TSubProgress<T> {mess};
      }
      
      ~SubProgresSenderClearer () {
	Messager{} << TSubProgress<std::string> {std::string("")} ;
      }  
    };
    
    class AsyncExecutor {
    public:
      template<class F, class... Args>
      auto execute (Messager& mess, F f,Args... args) {
	auto res = std::async(f,args...);
	std::future_status status;
	do {
	  status = res.wait_for(500ms);
	  mess.pumpProgress();
	}while(status != std::future_status::ready);
	return res.get();
      }
    };
    
  } // namespace Support
} // namespace MiniMC

#endif
