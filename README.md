# MiniMC

MiniMC is a model-checking tool for analyzing program behavior and reachability. It loads programs into an intermediate representation and provides concrete and symbolic program analyses. The `mc` subcommand checks reachability of assertion violations; its `--mc.symbolic` option selects symbolic analysis.

The C++23 project includes input front ends for MiniMC's own format, LLVM, and Whiley, plus program modeling, configurable analyses (CPAs), reachability algorithms, a simulator, and a virtual machine.

## Configurable program analyses

MiniMC uses configurable program analyses (CPAs) to define how program states and transitions are represented and evaluated. The `mc` command supports these analysis modes:

- **Concrete** (default): executes with concrete values and enumerates nondeterministic integer choices.
- **Symbolic** (`--mc.symbolic`): represents values as symbolic expressions and uses an SMT solver to check constraints and state feasibility.
- **Probabilistic/fuzzy concrete** (`--mc.fuzz N`): samples nondeterministic integer values using a configurable sample count `N`.

These are different analysis strategies, not guarantees that every program or input behavior is fully explored; the selected mode and input model determine what is analyzed.

## Instruction support

MiniMC's intermediate model includes operations for:

- Integer arithmetic, bitwise operations, shifts, and signed/unsigned comparisons.
- Integer, Boolean, and pointer casts, plus pointer arithmetic.
- Memory loads and stores, allocation and freeing, and pointer validity checks.
- Aggregate value insertion and extraction.
- Function calls and returns, along with assertions and assumptions used to express properties and constrain execution.

These are the model-level operation categories; they do not imply support for every instruction in LLVM IR. The supported input subset depends on the selected loader.

## Adding an instruction

When extending the instruction set:

1. Add the instruction's declarative definition to `libs/model/isa/opcodes.yaml`, following a nearby entry. Specify its opcode, operands, type constraints, and any relevant size constraints or flags. Some pseudo-instructions can be expressed in terms of existing operations with `template_construction`.
2. Keep generated headers and sources out of hand edits. CMake runs `isa2cpp.py` and `typecheckgen.py` to generate instruction declarations, constructors, expressions, and type-checker code from `opcodes.yaml` and the templates under `libs/model/isa/templates/`.
3. Implement execution semantics for the applicable value domains and VM operations (including concrete and symbolic domains when supported); ISA generation supplies declarations and type checking, not necessarily execution behavior.
4. Update any relevant loader mappings so input instructions lower to the new model instruction. For LLVM, use the mapping in `libs/loaders/llvm/`; MiniMC's own input parser consumes the model opcode names.
5. Add focused tests for type checking and execution, plus loader/program integration tests where applicable; then build and run the relevant CTest tests.

## Build

Configure and build with CMake:

```sh
cmake -S . -B build
cmake --build build
```

Some dependencies are fetched by CMake when not available locally. LLVM, Flex, Bison, and Boost.Program_options are used by project components; a compatible C++23 compiler and CMake are required.

## Tests

Run the configured test suite with CTest:

```sh
ctest --test-dir build --output-on-failure
```

## Usage

The command-line tool accepts an input file followed by a subcommand:

```text
minimc [OPTIONS] INPUT SUBCOMMAND
```

For symbolic reachability analysis, select the `mc` command and enable its symbolic option:

```sh
minimc [loader options] program.mc mc --mc.symbolic
```

The exact loader options and accepted input formats depend on the selected input. See `minimc --help` for available options and subcommands.

## Repository layout

- `bin/` — command-line executable and related tools.
- `libs/model/` — program intermediate representation and transformations.
- `libs/loaders/` — program input front ends.
- `libs/cpa/` and `libs/algorithms/` — analyses and reachability algorithms.
- `libs/simulator/` and `libs/vm/` — simulation and execution components.
- `tests/` — coded and program-based tests.
