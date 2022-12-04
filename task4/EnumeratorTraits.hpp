#pragma once

#include <type_traits>
#include <cstdint>
#include <array>
#include <string_view>
#include <numeric>

template<class Enum, std::size_t MAXN = 512> requires std::is_enum_v<Enum>
struct EnumeratorTraits {
    static constexpr std::size_t size() noexcept {
        return std::accumulate(valid_map.begin(), valid_map.end(), 0);
    }

    static constexpr Enum at(std::size_t i) noexcept {
        return values[i];
    }

    static constexpr std::string_view nameAt(std::size_t i) noexcept {
        return names[i];
    }

 private:
    using enum_type = std::underlying_type_t<Enum>;
    static constexpr auto generate_names() {
        return names_from_indices(std::make_index_sequence<size()>());
    }

    template<std::size_t... Is>
    static constexpr auto names_from_indices(std::index_sequence<Is...>) {
        return std::array<std::string_view, sizeof...(Is)>{name_at_value_at_index<Is>()...};
    }

    template<std::size_t index>
    static constexpr std::string_view name_at_value_at_index() {
        return name<at(index)>();
    }

    static constexpr auto generate_values() {
        return values_from_indices(std::make_index_sequence<size()>());
    }

    template<std::size_t... Is>
    static constexpr auto values_from_indices(std::index_sequence<Is...>) {
        constexpr auto valid = valid_map;
        constexpr auto values_count = sizeof...(Is);

        Enum values_storage[values_count];

        for (std::size_t i = 0, out_i = 0; out_i < values_count; ++i) {
            if (valid[i]) {
                values_storage[out_i++] = static_cast<Enum>(index_to_value(i));
            }
        }

        return std::array<Enum, values_count>{values_storage[Is]...};
    }

    static constexpr enum_type min_possible() {
        return std::max(static_cast<long long>(-MAXN),
                        static_cast<long long>(std::numeric_limits<enum_type>::min()));
    }

    static constexpr enum_type max_possible() {
        return std::min(MAXN, static_cast<std::size_t>(std::numeric_limits<enum_type>::max()));
    }

    static constexpr auto generate_valid_map() noexcept {
        constexpr std::size_t possible_values = max_possible() - min_possible() + 1;
        return valid_map_from_indices(std::make_index_sequence<possible_values>());
    }

    template<std::size_t... Is>
    static constexpr auto valid_map_from_indices(std::index_sequence<Is...>) noexcept {
        return std::array<bool, sizeof...(Is)>{index_valid<Is>()...};
    }

    template<std::size_t index>
    static constexpr bool index_valid() {
        return value_valid<static_cast<Enum>(index_to_value(index))>();
    }

    static constexpr enum_type index_to_value(std::size_t i) {
        return i + min_possible();
    }

    template<Enum e>
    static constexpr std::string_view name() {
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
    static constexpr bool value_valid() {
        return name<e>()[0] != '(';
    }

    // The order is as such because each next variables uses the previous one
    // `valid_map` -> used by `values` -> used by `names`
    inline static constexpr auto valid_map = EnumeratorTraits::generate_valid_map();
    inline static constexpr auto values = EnumeratorTraits::generate_values();
    inline static constexpr auto names = EnumeratorTraits::generate_names();
};
