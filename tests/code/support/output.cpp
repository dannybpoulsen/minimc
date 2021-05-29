#include "support/feedback.hpp"

int main () {
  auto& messager =  MiniMC::Support::getMessager ();//makeMessager (MiniMC::Support::MessagerType::Terminal);
  messager.message ("Hej");
  messager.error ("Error");
  messager.warning ("Warning");
}
  
