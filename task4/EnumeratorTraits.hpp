#pragma once

#include <type_traits>
#include <cstdint>
#include <array>
#include <string_view>
#include <numeric>
#include <algorithm>

template<class Enum, std::size_t MAXN = 512> requires std::is_enum_v<Enum>
struct EnumeratorTraits {
    static constexpr std::size_t size() noexcept {
        return size_;
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
        constexpr auto count = size();

        std::array<std::string_view, count> names{};

        for (std::size_t i = 0, out_i = 0; out_i < count; ++i) {
            if (valid_[i]) {
                names[out_i++] = all_names_[i];
            }
        }

        return names;
    }

    static constexpr auto GenerateValues() {
        constexpr auto count = size();

        std::array<Enum, count> values{};

        for (std::size_t i = 0, out_i = 0; out_i < count; ++i) {
            if (valid_[i]) {
                values[out_i++] = IndexToValue(i);
            }
        }

        return values;
    }

    static constexpr std::size_t CalculateSize() {
        return std::accumulate(valid_.begin(), valid_.end(), 0);
    }

    static constexpr auto GenerateValid() {
        std::array<bool, all_names_.size()> valid{};
        std::transform(all_names_.begin(), all_names_.end(), valid.begin(), NameValid);
        return valid;
    }

    static constexpr bool NameValid(std::string_view name) {
        return name[0] != '(';
    }

    static constexpr auto GenerateAllNames() {
        constexpr std::size_t possible_values = MaxPossible() - MinPossible() + 1;
        return AllNamesFromIndices(std::make_index_sequence<possible_values>());
    }

    static constexpr EnumType MinPossible() {
        return std::max(static_cast<long long>(-MAXN),
                        static_cast<long long>(std::numeric_limits<EnumType>::min()));
    }

    static constexpr EnumType MaxPossible() {
        return std::min(MAXN, static_cast<std::size_t>(std::numeric_limits<EnumType>::max()));
    }

    template<std::size_t... Is>
    static constexpr auto AllNamesFromIndices(std::index_sequence<Is...>) {
        return std::array<std::string_view, sizeof...(Is)>{NameAtIndex<Is>()...};
    }

    template<std::size_t index>
    static constexpr std::string_view NameAtIndex() {
        return Name<IndexToValue(index)>();
    }

    static constexpr Enum IndexToValue(std::size_t i) {
        return static_cast<Enum>(i + MinPossible());
    }

    template<Enum e>
    static constexpr std::string_view Name() {
        std::string_view pretty_name = __PRETTY_FUNCTION__;

#if defined(__clang__)
        auto start = pretty_name.rfind('=') + 2;
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

    // The order is as such because of some dependencies between variables
    static constexpr auto all_names_ = GenerateAllNames();
    static constexpr auto valid_ = GenerateValid();
    static constexpr auto size_ = CalculateSize();
    static constexpr auto values_ = GenerateValues();
    static constexpr auto names_ = GenerateNames();
};
