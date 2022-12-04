#pragma once

#include <type_traits>
#include <cstdint>
#include <array>
#include <string_view>
#include <numeric>

template<class Enum, std::size_t MAXN = 512> requires std::is_enum_v<Enum>
struct EnumeratorTraits {
    static constexpr std::size_t size() noexcept {
        return std::accumulate(valid_map_.begin(), valid_map_.end(), 0);
    }

    static constexpr Enum at(std::size_t i) noexcept {
        return values_[i];
    }

    static constexpr std::string_view nameAt(std::size_t i) noexcept {
        return names_[i];
    }

 private:
    using EnumType = std::underlying_type_t<Enum>;

    static constexpr auto GenerateNames() {
        return NamesFromIndices(std::make_index_sequence<size()>());
    }

    template<std::size_t... Is>
    static constexpr auto NamesFromIndices(std::index_sequence<Is...>) {
        return std::array<std::string_view, sizeof...(Is)>{NameAtValueAtIndex<Is>()...};
    }

    template<std::size_t index>
    static constexpr std::string_view NameAtValueAtIndex() {
        return Name<at(index)>();
    }

    static constexpr auto GenerateValues() {
        return ValuesFromIndices(std::make_index_sequence<size()>());
    }

    template<std::size_t... Is>
    static constexpr auto ValuesFromIndices(std::index_sequence<Is...>) {
        constexpr auto valid = valid_map_;
        constexpr auto values_count = sizeof...(Is);

        Enum values_storage[values_count];

        for (std::size_t i = 0, out_i = 0; out_i < values_count; ++i) {
            if (valid[i]) {
                values_storage[out_i++] = IndexToValue(i);
            }
        }

        return std::array<Enum, values_count>{values_storage[Is]...};
    }

    static constexpr EnumType MinPossible() {
        return std::max(static_cast<long long>(-MAXN),
                        static_cast<long long>(std::numeric_limits<EnumType>::min()));
    }

    static constexpr EnumType MaxPossible() {
        return std::min(MAXN, static_cast<std::size_t>(std::numeric_limits<EnumType>::max()));
    }

    static constexpr auto GenerateValidMap() {
        constexpr std::size_t possible_values = MaxPossible() - MinPossible() + 1;
        return ValidMapFromIndices(std::make_index_sequence<possible_values>());
    }

    template<std::size_t... Is>
    static constexpr auto ValidMapFromIndices(std::index_sequence<Is...>) {
        return std::array<bool, sizeof...(Is)>{IndexValid<Is>()...};
    }

    template<std::size_t index>
    static constexpr bool IndexValid() {
        return ValueValid<IndexToValue(index)>();
    }

    static constexpr Enum IndexToValue(std::size_t i) {
        return static_cast<Enum>(i + MinPossible());
    }

    template<Enum e>
    static constexpr std::string_view Name() {
        std::string_view pretty_name = __PRETTY_FUNCTION__;

#if defined(__clang__)
        auto start = pretty_name.rfind('=')  + 2;
        auto end = pretty_name.size() - 1;
#elif defined(__GNUC__)
        std::size_t start = 86;
        auto end = pretty_name.find(';', start);
#endif

        // Scoped enum case
        auto colon_position = pretty_name.rfind(':', end);

        if (colon_position > start) {
            start = colon_position + 1;
        }

        auto size = end - start;

        return pretty_name.substr(start, size);
    }

    template<Enum e>
    static constexpr bool ValueValid() {
        return Name<e>()[0] != '(';
    }

    // The order is as such because each next variables uses the previous one
    // `valid_map_` -> used by `values_` -> used by `names_`
    inline static constexpr auto valid_map_ = EnumeratorTraits::GenerateValidMap();
    inline static constexpr auto values_ = EnumeratorTraits::GenerateValues();
    inline static constexpr auto names_ = EnumeratorTraits::GenerateNames();
};
