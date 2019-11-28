#include <memory>
#include "support/types.hpp"

namespace MiniMC {
  namespace Storage {
    class BlobStorage {
    public:
      using BlobData_ptr = std::unique_ptr<MiniMC::uint8_t[]>;
      using BlobEntry = MiniMC::uint8_t*;
      virtual ~BlobStorage () {}
      virtual BlobEntry store (BlobData_ptr& ptr,MiniMC::offset_t) = 0;
      virtual void clear () = 0;
    };

    BlobStorage& getStorage ();
    
  }
}
