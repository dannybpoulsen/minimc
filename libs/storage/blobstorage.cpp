#include <memory>
#include <unordered_map>
#include "support/types.hpp"
#include "hash/hashing.hpp"
#include "storage/BlobStorage.hpp"

namespace MiniMC {
  namespace Storage {
    class DefaultStorage : public BlobStorage {
    public:
      virtual BlobEntry store (BlobData_ptr& ptr,MiniMC::offset_t size) {
	auto hash = MiniMC::Hash::Hash(ptr.get(),size,size);
	auto it = map.find (hash);
	if (it!=map.end()) {
	  return it->second.get();
	}
	else {
	  auto buf = ptr.get();
	  map.insert (std::make_pair (hash,std::move(ptr)));
	  return buf;
	}
      }

      void clear () {
	map.clear();
      }
      
    private:
      std::unordered_map<MiniMC::Hash::hash_t,BlobStorage::BlobData_ptr> map; 
    };
    
    BlobStorage& getStorage () {
      static std::unique_ptr<DefaultStorage> store = nullptr;
      if (!store)
	store.reset (new DefaultStorage ());
      return *store.get();
    }
    
    
  }
}
