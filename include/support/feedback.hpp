#ifndef _FEEDBACK__
#define _FEEDBACK__

#include <memory>
#include <gsl/pointers>


namespace MiniMC {
  namespace Support {

    
    
    class Progresser {
    public:
      virtual ~Progresser () {}
      virtual void progressMessage (const std::string& ) {}
    };
	
    using Progresser_ptr = std::unique_ptr<Progresser>;

	/**
	 *
	 * Messager objects is MiniMCs primary way of relaying messages
	 * from  algorithms to the user interface. If an algorithms wishes
	 * to do ProgressMessages then a Progresser object should be made
	 * by the Messager.  
	 * 
	 */
    class Messager {
    public:
	  virtual ~Messager () {}
      virtual Progresser_ptr makeProgresser () {return std::make_unique<Progresser> ();}; 
      virtual void error (const std::string&) {}
      virtual void warning (const std::string&) {};
      virtual void message (const std::string&) {};
    };

	using Messager_ptr = std::unique_ptr<Messager>;
	
	
    enum class MessagerType {
	  Terminal
    };

    void setMessager (MessagerType);
	
	Messager& getMessager ();
	
  }
}

#endif
