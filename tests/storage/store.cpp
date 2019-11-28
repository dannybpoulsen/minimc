#include "gtest/gtest.h"
#include "storage/BlobStorage.hpp"

TEST (Storage, InsertProper) { 
  auto& store = MiniMC::Storage::getStorage ();
  store.clear ();
  MiniMC::Storage::BlobStorage::BlobData_ptr data (new MiniMC::uint8_t[10]);
  std::fill (data.get(),data.get()+10,0);
  auto buf = data.get();
  EXPECT_EQ(store.store(data,10),buf);
}



TEST (Storage, Uniquenes) { 
  auto& store = MiniMC::Storage::getStorage ();
  store.clear ();
  MiniMC::Storage::BlobStorage::BlobData_ptr data (new MiniMC::uint8_t[10]);
  MiniMC::Storage::BlobStorage::BlobData_ptr data2 (new MiniMC::uint8_t[10]);

  std::fill (data2.get(),data2.get()+10,0);
  std::fill (data.get(),data.get()+10,0);
  auto buf = data.get();
  auto buf2 = data2.get();
  EXPECT_EQ(store.store(data,10),buf);
  EXPECT_EQ(store.store(data2,10),buf);
}

TEST (Storage, UniquenesTwo) { 
  auto& store = MiniMC::Storage::getStorage ();
  store.clear ();
  MiniMC::Storage::BlobStorage::BlobData_ptr data (new MiniMC::uint8_t[10]);
  MiniMC::Storage::BlobStorage::BlobData_ptr data2 (new MiniMC::uint8_t[10]);

  std::fill (data2.get(),data2.get()+10,0);
  std::fill (data.get(),data.get()+10,1);
  auto buf = data.get();
  auto buf2 = data2.get();
  EXPECT_EQ(store.store(data,10),buf);
  EXPECT_EQ(store.store(data2,10),buf2);
}
