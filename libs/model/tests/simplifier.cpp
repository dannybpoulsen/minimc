#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN


#include "doctest/doctest.h"


#include <minimc/model/variables.hpp>




TEST_CASE("AddExpr Simplification") {
  MiniMC::Model::ExprSimplifier simplifier;
  auto l = MiniMC::Model::I8Integer::make(5);
  auto r = MiniMC::Model::I8Integer::make(10);
  auto addr = MiniMC::Model::makeExpr<MiniMC::Model::AddExpr> (l,r);
  auto add_res = simplifier.Simplify (*addr);
  auto res   = add_res->template as<MiniMC::Model::I8Integer> ();

  CHECK (res.getValue() == 15);
}


TEST_CASE("AddExpr Simplification I16" ) {
  MiniMC::Model::ExprSimplifier simplifier;
  auto l = MiniMC::Model::I16Integer::make(5);
  auto r = MiniMC::Model::I16Integer::make(10);
  auto addr = MiniMC::Model::makeExpr<MiniMC::Model::AddExpr> (l,r);
  auto add_res = simplifier.Simplify (*addr);
  auto res   = add_res->template as<MiniMC::Model::I16Integer> ();

  CHECK (res.getValue() == 15);
}


