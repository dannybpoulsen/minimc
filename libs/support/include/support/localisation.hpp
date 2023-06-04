#ifndef _LOCALISATION__
#define _LOCALISATION__

#include <boost/format.hpp>

namespace MiniMC {
  namespace Support {
    
    
    class Localiser {
    public:
      Localiser(const std::string& str) : formatter(str) {}

      template <typename... Args>
      std::string format(Args&&... args) const {
        formatter.clear();
        (formatter.operator%(std::forward<Args>(args)), ...);
        return formatter.str();
      }

    private:
      mutable boost::format formatter;
    };
  } // namespace Support
} // namespace MiniMC

#endif
