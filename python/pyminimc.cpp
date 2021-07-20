#include <pybind11/pybind11.h>


namespace py = pybind11;

void addModelModule (py::module& m);
void addLoaderModule (py::module& m);



PYBIND11_MODULE(pyminimc, minimc) {
  minimc.def ("Test",[](){return "HEJ";});
  addModelModule (minimc);
  addLoaderModule (minimc);
}





