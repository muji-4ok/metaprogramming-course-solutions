#pragma once

#include <cstdint>
#include <utility>
#include <tuple>
#include <array>
#include <cassert>
#include <numeric>
#include <algorithm>
#include <functional>

template<typename...>
class Annotate {};

namespace {

template<std::size_t Index> // only for pack-expansion to work
struct AnyCastable {
    template<typename T>
    constexpr operator T() const noexcept;
};

template<typename T, typename... Args>
concept AggregateInitializableFrom = requires(Args... args) {
    T{args...};
};

template<typename T, std::size_t... Is>
constexpr std::size_t CountFieldsImpl(std::integer_sequence<std::size_t, Is...>) {
    return sizeof...(Is) - 1;
}

template<typename T, std::size_t... Is>
requires AggregateInitializableFrom<T, AnyCastable<Is>...>
constexpr std::size_t CountFieldsImpl(std::integer_sequence<std::size_t, Is...>) {
    return CountFieldsImpl<T>(std::index_sequence<0, Is...>());
}

template<typename T>
constexpr std::size_t
    CountFields = CountFieldsImpl<T>(std::make_integer_sequence<std::size_t, 0>());

template<typename T>
constexpr auto ConvertToTuple(T &value) {
    constexpr auto field_count = CountFields<T>;

    if constexpr (field_count == 0) {
        return std::tuple<>();
    } else if constexpr (field_count == 1) {
        auto &[v1] = value;
        return std::tie(v1);
    } else if constexpr (field_count == 2) {
        auto &[v1, v2] = value;
        return std::tie(v1, v2);
    } else if constexpr (field_count == 3) {
        auto &[v1, v2, v3] = value;
        return std::tie(v1, v2, v3);
    } else if constexpr (field_count == 4) {
        auto &[v1, v2, v3, v4] = value;
        return std::tie(v1, v2, v3, v4);
    } else if constexpr (field_count == 5) {
        auto &[v1, v2, v3, v4, v5] = value;
        return std::tie(v1, v2, v3, v4, v5);
    } else if constexpr (field_count == 6) {
        auto &[v1, v2, v3, v4, v5, v6] = value;
        return std::tie(v1, v2, v3, v4, v5, v6);
    } else if constexpr (field_count == 7) {
        auto &[v1, v2, v3, v4, v5, v6, v7] = value;
        return std::tie(v1, v2, v3, v4, v5, v6, v7);
    } else if constexpr (field_count == 8) {
        auto &[v1, v2, v3, v4, v5, v6, v7, v8] = value;
        return std::tie(v1, v2, v3, v4, v5, v6, v7, v8);
    } else if constexpr (field_count == 9) {
        auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9] = value;
        return std::tie(v1, v2, v3, v4, v5, v6, v7, v8, v9);
    } else if constexpr (field_count == 10) {
        auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10] = value;
        return std::tie(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
    } else if constexpr (field_count == 11) {
        auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11] = value;
        return std::tie(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11);
    } else if constexpr (field_count == 12) {
        auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12] = value;
        return std::tie(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12);
    } else if constexpr (field_count == 13) {
        auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13] = value;
        return std::tie(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13);
    } else if constexpr (field_count == 14) {
        auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14] = value;
        return std::tie(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14);
    } else if constexpr (field_count == 15) {
        auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15] = value;
        return std::tie(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15);
    } else if constexpr (field_count == 16) {
        auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16] = value;
        return std::tie(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16);
    }

    assert(false || "Struct has too many fields!");
}

template<typename T>
T declval();

template<typename T, std::size_t Index>
using FieldAtIndex = std::decay_t<std::tuple_element_t<Index,
                                                       decltype(ConvertToTuple<T>(declval<T &>()))>>;

template<typename T>
concept IsAnnotation = requires(T t) {
    []<typename... Args>(Annotate<Args...>) {}(t);
};

template<typename T, std::size_t... Is>
constexpr auto IsAnnotationMappingImpl(std::integer_sequence<std::size_t, Is...>) {
    return std::array<bool, sizeof...(Is)>{IsAnnotation<FieldAtIndex<T, Is>>...};
}

template<typename T>
constexpr auto IsAnnotationMapping() {
    return IsAnnotationMappingImpl<T>(std::make_integer_sequence<std::size_t, CountFields<T>>());
}

template<typename T, std::size_t TargetRealIndex>
consteval std::size_t GetIndexOfRealField() {
    constexpr auto is_annotation = IsAnnotationMapping<T>();

    for (std::size_t i = 0, real_index = -1; i < is_annotation.size(); ++i) {
        if (!is_annotation[i]) {
            if (++real_index == TargetRealIndex) {
                return i;
            }
        }
    }

    assert(false || "Target index too big!");
}

template<typename T>
consteval std::size_t CountRealFields() {
    constexpr auto is_annotation = IsAnnotationMapping<T>();
    return is_annotation.size() - std::accumulate(is_annotation.begin(), is_annotation.end(), 0);
}

template<typename T, std::size_t TargetRealIndex>
consteval std::size_t GetIndexOfFirstAnnotation() {
    if (TargetRealIndex == 0) {
        return 0;
    } else {
        return GetIndexOfRealField<T, TargetRealIndex - 1>() + 1;
    }
}

constexpr auto MergeAnnotationsImpl() {
    return Annotate<>{};
}

template<typename Annotation>
constexpr auto MergeAnnotationsImpl(Annotation a) {
    return a;
}

template<typename... Args1, typename... Args2>
constexpr auto MergeAnnotationsImpl(Annotate<Args1...>, Annotate<Args2...>) {
    return Annotate<Args1..., Args2...>{};
}

template<typename Annotation1, typename Annotation2, typename... AnnotationsTail>
constexpr auto MergeAnnotationsImpl(Annotation1 a1, Annotation2 a2, AnnotationsTail... a_tail) {
    return MergeAnnotationsImpl(MergeAnnotationsImpl(a1, a2), a_tail...);
}

template<typename... Annotations>
using MergeAnnotations = decltype(MergeAnnotationsImpl(declval<Annotations>()...));

template<typename T, std::size_t... Is>
constexpr auto MergeAnnotationsAtIndices(std::integer_sequence<std::size_t, Is...>) {
    return MergeAnnotations<FieldAtIndex<T, Is>...>{};
}

template<std::size_t Offset, std::size_t... Is>
consteval auto MakeOffsetIndexSequence(std::integer_sequence<std::size_t, Is...>) {
    return std::integer_sequence<std::size_t, Offset + Is...>();
}

template<typename T, std::size_t FieldIndex>
consteval auto MakeAnnotationsIndexSequence() {
    constexpr auto first_index = GetIndexOfFirstAnnotation<T, FieldIndex>();
    constexpr auto count = GetIndexOfRealField<T, FieldIndex>() - first_index;
    return MakeOffsetIndexSequence<first_index>(std::make_integer_sequence<std::size_t, count>());
}

template<typename T, std::size_t FieldIndex>
using GetAnnotationsForIndex = decltype(MergeAnnotationsAtIndices<T>(MakeAnnotationsIndexSequence<T,
                                                                                                  FieldIndex>()));

template<typename Target, typename... Annotations>
constexpr auto IsAnnotationClassMapping(Annotate<Annotations...>) {
    return std::array<bool, sizeof...(Annotations)>{std::is_same_v<Target, Annotations>...};
}

template<typename Target, typename Annotation>
constexpr bool HasAnnotationClass() {
    return std::ranges::any_of(IsAnnotationClassMapping<Target>(Annotation{}), std::identity{});
}

template<typename T, template<typename...> typename Template>
struct IsTemplateOfImpl {
    static constexpr bool value = false;
};

template<template<typename...> typename Template, typename... Args>
struct IsTemplateOfImpl<Template<Args...>, Template> {
    static constexpr bool value = true;
};

template<typename T, template<typename...> typename Template>
constexpr bool IsTemplateOf = IsTemplateOfImpl<T, Template>::value;

template<template<typename...> typename TargetTemplate, typename... Annotations>
constexpr auto IsAnnotationTemplateMapping(Annotate<Annotations...>) {
    return std::array<bool, sizeof...(Annotations)>{
        IsTemplateOf<Annotations, TargetTemplate>...};
}

template<template<typename...> typename TargetTemplate, typename CompositeAnnotation>
constexpr bool HasAnnotationTemplate() {
    return std::ranges::any_of(IsAnnotationTemplateMapping<TargetTemplate>(CompositeAnnotation{}),
                               std::identity{});
}

template<template<typename...> typename TargetTemplate, typename CompositeAnnotation>
constexpr std::size_t GetIndexOfAnnotationTemplate() {
    constexpr auto mapping = IsAnnotationTemplateMapping<TargetTemplate>(CompositeAnnotation{});
    auto it = std::ranges::find(mapping, true);
    return it ? std::ranges::distance(std::ranges::begin(mapping), it) : mapping.size();
}

template<std::size_t Index, typename Annotation>
struct GetAnnotationArgAtIndex {
};

template<std::size_t Index, template<typename...> typename Annotation, typename... Args> requires (
    Index < sizeof...(Args))
struct GetAnnotationArgAtIndex<Index, Annotation<Args...>> {
    using type = std::tuple_element_t<Index, std::tuple<Args...>>;
};

template<template<typename...> typename TargetTemplate, typename CompositeAnnotation>
using FindAnnotationForTemplate = typename GetAnnotationArgAtIndex<GetIndexOfAnnotationTemplate<
    TargetTemplate,
    CompositeAnnotation>(), CompositeAnnotation>::type;

} // namespace

template<typename T, std::size_t Index>
struct FieldDescriptor {
    using Type = FieldAtIndex<T, GetIndexOfRealField<T, Index>()>;
    using Annotations = GetAnnotationsForIndex<T, Index>;

    template<template<typename...> typename AnnotationTemplate>
    static constexpr bool
        has_annotation_template = HasAnnotationTemplate<AnnotationTemplate, Annotations>();

    template<typename Annotation>
    static constexpr bool has_annotation_class = HasAnnotationClass<Annotation, Annotations>();

    template<template<typename...> class AnnotationTemplate>
    using FindAnnotation = FindAnnotationForTemplate<AnnotationTemplate, Annotations>;
};

template<typename T>
struct Describe {
    static constexpr std::size_t num_fields = CountRealFields<T>();

    template<std::size_t I>
    using Field = FieldDescriptor<T, I>;
};
