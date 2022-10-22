#pragma once

#include <optional>

template<class FromIn, auto target>
struct Mapping {
  using From = FromIn;
  static constexpr decltype(target) Value = target;
};

template<class Base, class Target, class... Mappings>
struct PolymorphicMapper {
};

template<class Base, class Target, class MappingsHead, class... MappingsTail> requires
std::same_as<std::remove_cv_t<decltype(MappingsHead::Value)>, Target> &&
    std::derived_from<typename MappingsHead::From, Base>
struct PolymorphicMapper<Base, Target, MappingsHead, MappingsTail...> {
  static std::optional<Target> map(const Base &object) {
    using From = typename MappingsHead::From;

    if (dynamic_cast<const From *>(&object)) {
      return MappingsHead::Value;
    } else {
      return PolymorphicMapper<Base, Target, MappingsTail...>::map(object);
    }
  }
};

template<class Base, class Target>
struct PolymorphicMapper<Base, Target> {
  static std::optional<Target> map(const Base &) {
    return std::nullopt;
  }
};
