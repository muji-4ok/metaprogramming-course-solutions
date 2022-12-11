#pragma once

#include <concepts>

#include <type_tuples.hpp>

namespace type_lists {

template<class TL>
concept TypeSequence = requires {
  typename TL::Head;
  typename TL::Tail;
};

struct Nil {};

template<class TL>
concept Empty = std::derived_from<TL, Nil>;

template<class TL>
concept TypeList = Empty<TL> || TypeSequence<TL>;

template<typename T>
struct Repeat {
  using Head = T;
  using Tail = Repeat<T>;
};

template<typename InHead, typename InTail>
struct Cons {
  using Head = InHead;
  using Tail = InTail;
};

namespace {

template<TypeList TL, typename... Prev>
struct ToTupleImpl {
  using Tuple = typename ToTupleImpl<typename TL::Tail, Prev..., typename TL::Head>::Tuple;
};

template<Empty E, typename... Prev>
struct ToTupleImpl<E, Prev...> {
  using Tuple = type_tuples::TTuple<Prev...>;
};

// ** HEAD **
template<TypeList TL>
struct HeadImpl {
  using TLOut = typename TL::Head;
};

template<Empty E>
struct HeadImpl<E> {
  using TLOut = E;
};

template<TypeList TL>
using GetHead = typename HeadImpl<TL>::TLOut;

// ** TAIL **
template<TypeList TL>
struct TailImpl {
  using TLOut = typename TL::Tail;
};

template<Empty E>
struct TailImpl<E> {
  using TLOut = E;
};

template<TypeList TL>
using GetTail = typename TailImpl<TL>::TLOut;

} // namespace

template<TypeList TL>
using ToTuple = typename ToTupleImpl<TL>::Tuple;

template<typename TypeTuple>
struct FromTuple {
};

template<typename First, typename... Others>
struct FromTuple<type_tuples::TTuple<First, Others...>> {
  using Head = First;
  using Tail = FromTuple<type_tuples::TTuple<Others...>>;
};

template<>
struct FromTuple<type_tuples::TTuple<>> : Nil {
};

template<std::size_t N, TypeList TL>
struct Take : TL {
  using Tail = Take<N - 1, typename TL::Tail>;
};

template<TypeList TL>
struct Take<0, TL> : Nil {
};

template<std::size_t N, Empty E>
struct Take<N, E> : Nil {
};

template<std::size_t N, TypeList TL>
struct Drop : Drop<N - 1, typename TL::Tail> {
};

template<TypeList TL>
struct Drop<0, TL> : TL {
};

template<std::size_t N, Empty E>
struct Drop<N, E> : E {
};

template<std::size_t N, typename T>
using Replicate = Take<N, Repeat<T>>;

template<template<typename> typename MetaFunc, typename T>
struct Iterate {
  using Head = T;
  using Tail = Iterate<MetaFunc, MetaFunc<T>>;
};

template<TypeSequence FullCycle, TypeList PartOfCycle = Nil>
struct Cycle {
  using Head = typename PartOfCycle::Head;
  using Tail = Cycle<FullCycle, Drop<1, PartOfCycle>>;
};

template<TypeSequence FullCycle, Empty E>
struct Cycle<FullCycle, E> {
  using Head = typename FullCycle::Head;
  using Tail = Cycle<FullCycle, Drop<1, FullCycle>>;
};

template<template<typename> typename MetaFunc, TypeList TL>
struct Map {
  using Head = MetaFunc<typename TL::Head>;
  using Tail = Map<MetaFunc, typename TL::Tail>;
};

template<template<typename> typename MetaFunc, Empty E>
struct Map<MetaFunc, E> : E {
};

template<template<typename> typename MetaPredicate, TypeList TL>
struct Filter {
  using Head = typename TL::Head;
  using Tail = Filter<MetaPredicate, typename TL::Tail>;
};

template<template<typename> typename MetaPredicate, TypeList TL> requires (!MetaPredicate<typename TL::Head>::Value)
struct Filter<MetaPredicate, TL> : Filter<MetaPredicate, typename TL::Tail> {
};

template<template<typename> typename MetaPredicate, Empty E>
struct Filter<MetaPredicate, E> : E {
};

template<template<typename, typename> typename MetaOp, typename StartType, TypeList TL>
struct Scanl {
  using Head = StartType;
  using Tail = Scanl<MetaOp, MetaOp<StartType, typename TL::Head>, typename TL::Tail>;
};

template<template<typename, typename> typename MetaOp, typename StartType, Empty E>
struct Scanl<MetaOp, StartType, E> {
  using Head = StartType;
  using Tail = E;
};

namespace {

template<template<typename, typename> typename MetaOp, typename StartType, TypeList TL>
struct FoldlImpl {
  using Type = typename FoldlImpl<MetaOp,
                                  MetaOp<StartType, typename TL::Head>,
                                  typename TL::Tail>::Type;
};

template<template<typename, typename> typename MetaOp, typename StartType, Empty E>
struct FoldlImpl<MetaOp, StartType, E> {
  using Type = StartType;
};

}

template<template<typename, typename> typename MetaOp, typename StartType, TypeList TL>
using Foldl = typename FoldlImpl<MetaOp, StartType, TL>::Type;

namespace {

template<Empty E>
constexpr std::size_t Size() {
  return 0;
}

template<TypeList TL>
consteval std::size_t Size() {
  return 1 + Size<typename TL::Tail>();
}

} // namespace

template<TypeList TL, std::size_t N = 0>
struct Inits {
  using Head = Take<N, TL>;
  using Tail = Inits<TL, N + 1>;
};

template<TypeList TL, std::size_t N> requires (N > 0) && (Size<Take<N, TL>>() == N - 1)
struct Inits<TL, N> : Nil {
};

template<TypeList TL>
struct Tails {
  using Head = TL;
  using Tail = Tails<typename TL::Tail>;
};

template<Empty E>
struct Tails<E> {
  using Head = E;
  using Tail = E;
};

template<TypeList Left, TypeList Right>
struct Zip2 {
  using Head = type_tuples::TTuple<typename Left::Head, typename Right::Head>;
  using Tail = Zip2<typename Left::Tail, typename Right::Tail>;
};

template<Empty E, TypeList Right>
struct Zip2<E, Right> : E {
};

template<TypeList Left, Empty E>
struct Zip2<Left, E> : E {
};

template<TypeList... TLs>
struct Zip {
  using Head = type_tuples::TTuple<typename TLs::Head...>;
  using Tail = Zip<typename TLs::Tail...>;
};

} // namespace type_lists
