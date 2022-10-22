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

template<typename... Types>
struct TLFromTypes {
};

template<typename First, typename... Others>
struct TLFromTypes<First, Others...> {
  using TL = Cons<First, typename TLFromTypes<Others...>::TL>;
};

template<>
struct TLFromTypes<> {
  using TL = Nil;
};

template<typename... Types>
struct TLFromTypes<type_tuples::TTuple<Types...>> {
  using TL = typename TLFromTypes<Types...>::TL;
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
using Head = typename HeadImpl<TL>::TLOut;

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
using Tail = typename TailImpl<TL>::TLOut;

template<std::size_t N, TypeList TL>
struct TakeImpl {
  using TLOut = Cons<Head<TL>, typename TakeImpl<N - 1, Tail<TL>>::TLOut>;
};

template<std::size_t N, Empty E>
struct TakeImpl<N, E> {
  using TLOut = E;
};

template<TypeList TL>
struct TakeImpl<0, TL> {
  using TLOut = Nil;
};

template<std::size_t N, TypeList TL>
struct DropImpl {
  using TLOut = Tail<typename DropImpl<N - 1, TL>::TLOut>;
};

template<TypeList TL>
struct DropImpl<0, TL> {
  using TLOut = TL;
};

}

template<TypeList TL>
using ToTuple = typename ToTupleImpl<TL>::Tuple;

template<type_tuples::TypeTuple TypeTuple>
using FromTuple = typename TLFromTypes<TypeTuple>::TL;

template<std::size_t N, TypeList TL>
using Take = typename TakeImpl<N, TL>::TLOut;

template<std::size_t N, TypeList TL>
using Drop = typename DropImpl<N, TL>::TLOut;

template<std::size_t N, typename T>
using Replicate = Take<N, Repeat<T>>;

template<template<typename> typename MetaFunc, typename T>
struct Iterate {
  using Head = T;
  using Tail = Iterate<MetaFunc, MetaFunc<T>>;
};

namespace {

template<TypeSequence FullCycle, TypeList PartOfCycle>
struct CycleImpl {
  using Head = typename PartOfCycle::Head;
  using Tail = CycleImpl<FullCycle, Drop<1, PartOfCycle>>;
};

template<TypeSequence FullCycle, Empty E>
struct CycleImpl<FullCycle, E> {
  using Head = typename FullCycle::Head;
  using Tail = CycleImpl<FullCycle, Drop<1, FullCycle>>;
};

}

template<TypeSequence TL>
using Cycle = CycleImpl<TL, Nil>;

template<template<typename> typename MetaFunc, TypeList TL>
struct Map {
  using Head = MetaFunc<typename TL::Head>;
  using Tail = Map<MetaFunc, typename TL::Tail>;
};

template<template<typename> typename MetaFunc, Empty E>
struct Map<MetaFunc, E> : public E {
};

template<template<typename> typename MetaPredicate, TypeList TL>
struct Filter {
};

template<template<typename> typename MetaPredicate, TypeSequence TL> requires MetaPredicate<typename TL::Head>::Value
struct Filter<MetaPredicate, TL> {
  using Head = typename TL::Head;
  using Tail = Filter<MetaPredicate, typename TL::Tail>;
};

template<template<typename> typename MetaPredicate, TypeSequence TL> requires (!MetaPredicate<
    typename TL::Head>::Value)
struct Filter<MetaPredicate, TL> {
  using Next = Filter<MetaPredicate, typename TL::Tail>;
  using Head = Head<Next>;
  using Tail = Tail<Next>;
};

template<template<typename> typename MetaPredicate, Empty E>
struct Filter<MetaPredicate, E> : public E {
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

template<std::size_t N, TypeList TL>
struct InitsImpl {
  using Head = Take<N, TL>;
  using Tail = InitsImpl<N + 1, TL>;
};

template<std::size_t N, TypeList TL> requires (std::same_as<Take<N, TL>, TL>)
struct InitsImpl<N, TL> {
  using Head = Take<N, TL>;
  using Tail = Nil;
};

}

template <TypeList TL>
using Inits = InitsImpl<0, TL>;

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

template <TypeList Left, TypeList Right>
struct Zip2 {
  using Head = type_tuples::TTuple<typename Left::Head, typename Right::Head>;
  using Tail = Zip2<typename Left::Tail, typename Right::Tail>;
};

template <Empty E, TypeList Right>
struct Zip2<E, Right> : public E {
};

template <TypeList Left, Empty E>
struct Zip2<Left, E> : public E {
};

template <TypeList... TLs>
struct Zip {
  using Head = type_tuples::TTuple<typename TLs::Head...>;
  using Tail = Zip<typename TLs::Tail...>;
};

} // namespace type_lists
