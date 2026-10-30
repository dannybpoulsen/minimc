# Repository guide

MiniMC is a C++ model-checking tool for analyzing program behavior and reachability. It loads programs through front ends such as LLVM and Whiley, represents them in an intermediate model, and supports concrete and symbolic analyses.

## Project layout

- `bin/` contains command-line tools and the interpreter.
- `libs/model/` defines the intermediate representation and related program analyses and transformations.
- `libs/loaders/` contains program input front ends, including LLVM and Whiley.
- `libs/cpa/` implements configurable program analyses; `libs/algorithms/` provides reachability algorithms.
- `libs/simulator/` and `libs/vm/` provide simulation and execution components.
- `tests/` contains program-based tests; several libraries also keep tests alongside their sources.
- `dependencies/` declares external CMake dependencies.

## Build and test

The project uses CMake and C++23. Configure and build with:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Run the configured test suite with:

```sh
ctest --test-dir build --output-on-failure
```

CMake fetches some dependencies when they are not already available. LLVM, Flex, and Bison are also used by the loaders and simulator; consult the configure output if dependency discovery fails.

## Change guidance

- Follow the existing C++ conventions and `.clang-format` for formatting.
- Keep changes localized to the relevant library, tool, or test. Check nearby CMake files when adding sources or tests.
- For ISA changes, inspect the descriptions and templates under `libs/model/isa/` and use the existing generation workflow rather than editing generated files by hand.
- Do not commit build outputs, dependency checkouts, or editor caches such as `.cache/`.
