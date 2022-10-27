#include <limits>

namespace MiniMC {
  namespace Support {
    template <typename T>
    struct Generator {
      bool hasOne() {
        return !isLast;
      }

      T getVal() const {

        return t;
      }

      void advance() {
        if (t == std::numeric_limits<T>::max())
          isLast = true;

        t++;
      }

    private:
      T t = std::numeric_limits<T>::min();
      mutable bool isLast = false;
    };
  } // namespace Support
} // namespace MiniMC
