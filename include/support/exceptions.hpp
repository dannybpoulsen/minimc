#ifndef _EXCEPTION__
#define _EXCEPTION__

#include <stdexcept>
#include "support/localisation.hpp"

namespace MiniMC {
  namespace Support {
    class Exception : public std::runtime_error {
    public:
      Exception (const std::string& mess) : std::runtime_error(mess) {}
    };
	
    class VerificationException : public Exception {
    public:
      VerificationException (const std::string& mess) : Exception(mess) {}
    };

	class ConfigurationException : public Exception {
    public:
      ConfigurationException (const std::string& mess) : Exception(mess) {}
    };
	
    class BufferOverflow : public VerificationException {
    public:
      BufferOverflow () : VerificationException ("BUffer Overflow") {}
    };

    class InvalidPointer : public  VerificationException {
    public:
      InvalidPointer () : VerificationException ("Invalid Pointer") {}
    };
    
    class AssertViolated: public  VerificationException {
    public:
      AssertViolated () : VerificationException ("Assert Violation") {}
    };

    class AssumeViolated: public  VerificationException {
    public:
      AssumeViolated () : VerificationException ("Assume Violated") {}
    };

	class FunctionDoesNotExist : public  VerificationException {
    public:
      FunctionDoesNotExist (const std::string& name) : VerificationException (Localiser ("Function '%1%' does not exists").format(name)) {}
    };
	
    
  }
}

#endif 
