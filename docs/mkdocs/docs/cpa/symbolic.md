# Path Formula CPA  
The Path Formula CPA executes  statements of a program
symbolically while exploring a program path. It captures path-constraints over program registers in a
so-called path formula.
After reaching a place of interest a high-level algorithm can
evaluate whether the path is feasible by checking satisfiability of
the path constraint.

## Solvers  

Current {{minimc}} employ the following SMT solvers for solving path formulas:  

- [CVC4](https://cvc4.github.io/)  
