#ifndef _FEEDBACK__
#define _FEEDBACK__

#include <memory>
#include <gsl/pointers>


namespace MiniMC {
  namespace Support {

    
    
    class Progresser {
    public:
      virtual ~Progresser () {}
      virtual void progressMessage (const std::string& ) = 0; 
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
      virtual Progresser_ptr makeProgresser () = 0; 
      virtual void error (const std::string&) = 0;
      virtual void warning (const std::string&) = 0;
      virtual void message (const std::string&) = 0;
    };

    enum class MessagerType {
			 Terminal
    };

    using Messager_ptr = std::unique_ptr<Messager>;

    Messager_ptr makeMessager (MessagerType);
    
  }
}

#endif
