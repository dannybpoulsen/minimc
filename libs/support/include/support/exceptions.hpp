#ifndef _EXCEPTION__
#define _EXCEPTION__

#include "support/localisation.hpp"
#include <stdexcept>

namespace MiniMC {
  namespace Support {
    class Exception : public std::runtime_error {
    public:
      Exception(const std::string& mess) : std::runtime_error(mess) {}
    };

    class VerificationException : public Exception {
    public:
      VerificationException(const std::string& mess) : Exception(mess) {}
    };

    class ConfigurationException : public Exception {
    public:
      ConfigurationException(const std::string& mess) : Exception(mess) {}
    };

    class BufferOverflow : public VerificationException {
    public:
      BufferOverflow() : VerificationException("BUffer Overflow") {}
    };

    class BufferOverread : public MiniMC::Support::Exception {
    public:
      BufferOverread () : MiniMC::Support::Exception (MiniMC::Support::Localiser{"BufferOverread detected"}.format ()) {}
    };
    
    
    class InvalidFree : public MiniMC::Support::Exception {
    public:
      InvalidFree () : MiniMC::Support::Exception (MiniMC::Support::Localiser{"Invalid free detected"}.format ()) {}
    };
    
    
    class InvalidPointer : public VerificationException {
    public:
      InvalidPointer() : VerificationException("Invalid Pointer") {}
    };

    class AssertViolated : public VerificationException {
    public:
      AssertViolated() : VerificationException("Assert Violation") {}
    };

    class AssumeViolated : public VerificationException {
    public:
      AssumeViolated() : VerificationException("Assume Violated") {}
    };

    class FunctionDoesNotExist : public VerificationException {
    public:
      FunctionDoesNotExist(const std::string& name) : VerificationException(Localiser("Function '%1%' does not exists").format(name)) {}
    };
    

    	  

    
  } // namespace Support
} // namespace MiniMC

#endif
