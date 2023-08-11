#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN


#include "doctest/doctest.h"


#include <support/pipes.hpp>

struct Data {
  int k{0};
};

struct AddOne : public MiniMC::Support::Transformer<Data> {
  Data operator() (Data&& k) {
    k.k+=1;
    return k;
  }
};

TEST_CASE("Pipes") {
  MiniMC::Support::TransformManager<Data> manager;
  manager.add<AddOne> ();
  manager.add<AddOne> ();
  
  CHECK (manager (Data{}).k == 2);
}
