#ifndef _HOST__
#define _HOST__

namespace MiniMC {
  namespace Host {
    enum class ExitCodes {
      AllGood = 0,
      RuntimeError = 1,
      ConfigurationError = 2,
      UnexpectedResult = 4
    };

  }
} // namespace MiniMC

#endif
