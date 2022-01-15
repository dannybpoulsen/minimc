#include <iostream>
#include "support/bitgen.hpp"

int main () {
  MiniMC::Support::Generator<unsigned char> gen;
  while (gen.hasOne()) {
	std::cerr << (int) gen.getVal() << std::endl;
	gen.advance ();
  }
}
