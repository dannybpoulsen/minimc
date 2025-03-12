#ifndef _FEEDBACK__
#define _FEEDBACK__


#include <memory>
#include <sstream>
#include <type_traits>

#include <future>
#include <list>
#include <iosfwd>

using namespace std::chrono_literals;
using namespace std::chrono_literals;


namespace MiniMC {
  namespace Support {

    enum class Severity {
      Error,
      Warning,
      Info,
      Progress,
    };

    
    
    class Message {
    public:
      Message ()  {} 
      virtual ~Message () {}
      virtual std::ostream& to_string (std::ostream& ) const = 0;
      virtual Severity getType () const = 0;
    };

    template<class M>
    concept HasToString = requires (const M& m, std::ostream& os) {
      {m.to_string (os)};
    };
    
    
    template<HasToString M>
    inline std::ostream& operator<< (std::ostream& os, const M& m) {
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
    
    template<class M>
    concept Outputtable = requires (const M& m, std::ostream& os) {
      {os << m};
    };
    
    template<Outputtable T, Severity t>
    class TMessage : public MessageT<t> {
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

    
    
    
    class MessageSink {
    public:
      virtual ~MessageSink() {}
      virtual void mess(const Message&) {}
      virtual void pumpProgress() {}
      
      static std::shared_ptr<MessageSink> defaultSink ();
      static void setDefaultSink (std::shared_ptr<MessageSink>);
      
    };
    
    class MessageHandler {
    public:
      virtual bool handle (const Message& ) = 0;
      virtual void pump () = 0;
    };

    class MessagePipeline : public MessageSink {
    public:
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
      
      
      
    private:
      std::list<std::unique_ptr<MessageHandler>> handlers;
    };

    class MessagePipelineBuilder {
    public:
      template<class T,class... Args>
      auto add (Args&&... args) {
	handlers.push_back (std::make_unique<T>(std::forward<Args>(args)...));
      }

      std::shared_ptr<MessagePipeline> build() {
	return std::make_shared<MessagePipeline> (std::move(handlers));
      }
      
    private:
      std::list<std::unique_ptr<MessageHandler>> handlers;
      
    };

    template<Severity sev>
    class StreamHandler : public MessageHandler {
    public:
      StreamHandler (std::ostream& o) : stream(o) {}
      void pump () override;
      bool handle (const Message&) override;
      
      
    private:
      std::ostream& stream;
    };


    
    
    class Messager {
    public:
      Messager (std::shared_ptr<MessageSink> sink = MessageSink::defaultSink ()) : sink(std::move(sink)) {}
      
      
      template<class T>
      auto& operator<< (T&& mess) requires std::derived_from<T,Message> {
	sink->mess(mess);
	return *this;
      }

      template<Outputtable T,Severity t= Severity::Info>
      auto& operator<< (T&& inp) requires (!std::derived_from<T,Message>) {
	return (*this << TMessage<T,t> {std::forward<T>(inp)});
      }

      
      void pumpProgress () {sink->pumpProgress();}
    private:
      std::shared_ptr<MessageSink> sink; 
      
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
