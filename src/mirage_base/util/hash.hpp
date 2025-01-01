#ifndef MIRAGE_BASE_UTIL_HASH
#define MIRAGE_BASE_UTIL_HASH

#include <concepts>
#include <cstddef>

namespace mirage::base {

template <typename T>  // NOLINT: Unused empty type.
struct Hash {};

template <typename T>
concept HashKeyType =
    std::move_constructible<T> && std::equality_comparable<T> &&
    std::move_constructible<Hash<T>> && std::copy_constructible<Hash<T>> &&
    std::default_initializable<Hash<T>> && requires(Hash<T> hasher, T val) {
      { hasher(val) } -> std::same_as<size_t>;
    };

template <>
struct Hash<size_t> {
  size_t operator()(const size_t val) const { return val; }
};

}  // namespace mirage::base

#endif  // MIRAGE_BASE_UTIL_HASH
