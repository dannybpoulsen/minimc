#ifndef MINIMC_EXPRESSION_SIMPLIFIER_DISPATCH_HPP
#define MINIMC_EXPRESSION_SIMPLIFIER_DISPATCH_HPP

#include "minimc/model/variables.hpp"

#include <functional>
#include <utility>

namespace MiniMC::Model {

template <class F>
Value_ptr foldMatchingIntegerOperands(Value& lhs, Value& rhs, F&& fold) {
  switch (lhs.type_t()) {
  case ValueInfo::type_t<I8Integer>():
    if (rhs.type_t() == ValueInfo::type_t<I8Integer>())
      return std::invoke(std::forward<F>(fold), lhs.as<I8Integer>(), rhs.as<I8Integer>());
    break;
  case ValueInfo::type_t<I16Integer>():
    if (rhs.type_t() == ValueInfo::type_t<I16Integer>())
      return std::invoke(std::forward<F>(fold), lhs.as<I16Integer>(), rhs.as<I16Integer>());
    break;
  case ValueInfo::type_t<I32Integer>():
    if (rhs.type_t() == ValueInfo::type_t<I32Integer>())
      return std::invoke(std::forward<F>(fold), lhs.as<I32Integer>(), rhs.as<I32Integer>());
    break;
  case ValueInfo::type_t<I64Integer>():
    if (rhs.type_t() == ValueInfo::type_t<I64Integer>())
      return std::invoke(std::forward<F>(fold), lhs.as<I64Integer>(), rhs.as<I64Integer>());
    break;
  default:
    break;
  }
  return nullptr;
}

template <class Lhs, class Rhs, class F>
Value_ptr foldOperands(Value& lhs, Value& rhs, F&& fold) {
  if (lhs.type_t() != ValueInfo::type_t<Lhs>() ||
      rhs.type_t() != ValueInfo::type_t<Rhs>())
    return nullptr;
  return std::invoke(std::forward<F>(fold), lhs.as<Lhs>(), rhs.as<Rhs>());
}

} // namespace MiniMC::Model

#endif
