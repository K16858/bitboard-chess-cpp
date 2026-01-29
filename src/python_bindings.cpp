#include "movegen.hpp"
#include <pybind11/pybind11.h>

namespace py = pybind11;

PYBIND11_MODULE(chess_engine, m) {
    m.doc() = "Chess engine with MCTS (pybind11 binding)";

    m.def("init", &MoveGen::Init, "Initialize move generator tables. Call once before using Board or run_mcts.");
}
