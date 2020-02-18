#include <iostream>
#include "support/random.hpp"

int main () {
  MiniMC::Support::RandomNumber number;
  for (size_t i = 0; i < 100; i++) {
    std::cerr << number.template uniform<int> (0,10) << "\n";
  }
}
