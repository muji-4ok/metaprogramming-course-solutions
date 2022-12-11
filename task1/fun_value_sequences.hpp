#pragma once

#include <value_types.hpp>
#include <type_lists.hpp>
#include <commons/metafunctions.hpp>

namespace {

template<typename T>
using MetaAddOne = value_types::ValueTag<T::Value + 1>;

}

using Nats = type_lists::Iterate<MetaAddOne, value_types::ValueTag<0>>;

namespace {

template<typename T>
struct FibAddImpl {
};

template<typename T, typename U>
struct FibAddImpl<type_tuples::TTuple<T, U>> {
  using Result = type_tuples::TTuple<value_types::ValueTag<U::Value>,
                                     value_types::ValueTag<T::Value + U::Value>>;
};

template<typename T>
using FibAdd = typename FibAddImpl<T>::Result;

template<typename T>
struct TakeFirstImpl {
};

template<typename T, typename U>
struct TakeFirstImpl<type_tuples::TTuple<T, U>> {
  using Result = T;
};

template<typename T>
using TakeFirst = typename TakeFirstImpl<T>::Result;

}

using Fib = type_lists::Map<TakeFirst, type_lists::Iterate<FibAdd,
                                                           type_tuples::TTuple<value_types::ValueTag<
                                                               0>,
                                                                               value_types::ValueTag<
                                                                                   1>>>>;

namespace {

template<typename T, typename U>
struct DividedBy {
  static constexpr bool Value = (T::Value % U::Value) == 0;
};

using NatsDividers = type_lists::Drop<2, Nats>;

template<typename T>
struct IsPrime {
  template<typename U>
  using Predicate = DividedBy<T, U>;

  using Dividers = type_lists::Take<T::Value - 2, NatsDividers>;

  using Filtered = type_lists::Filter<Predicate, Dividers>;

  static constexpr bool Value = type_lists::Empty<Filtered>;
};

}

using Primes = type_lists::Filter<IsPrime, NatsDividers>;
