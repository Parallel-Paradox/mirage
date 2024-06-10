#ifndef MIRAGE_BASE_UTIL_HASH
#define MIRAGE_BASE_UTIL_HASH

#include <concepts>
#include <cstddef>

#include "mirage_base/define.hpp"

namespace mirage {

template <typename T>
struct Hash {};

template <typename T>
concept HashKeyType =
    std::move_constructible<T> && std::equality_comparable<T> &&
    requires(Hash<T> hasher, T val) {
  { hasher(val) } -> std::same_as<size_t>;
};

template <>
struct Hash<size_t> {
  size_t operator()(size_t val) const { return val; }
};

}  // namespace mirage

#endif  // MIRAGE_BASE_UTIL_HASH
