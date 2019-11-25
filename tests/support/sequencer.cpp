#include <iostream>

#include "support/sequencer.hpp"

int main () {
  int i = 0;
  struct Add : public MiniMC::Support::Sink<int> {
    Add (int k) : i(k) {}
    virtual void run (int& t) override  {t=i+t;};
    int i;
  };

  struct MulTwo : public MiniMC::Support::Sink<int> {
    virtual void run (int& t) override  {t=t*2;};
  };

  MiniMC::Support::Sequencer<int> K;
  K.add<Add> (5)
    .add<MulTwo> ();

  K.run (i);
  std::cerr << i << std::endl;
  
}
