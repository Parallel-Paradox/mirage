#ifndef MIRAGE_BASE_CONTAINER_HASH_MAP
#define MIRAGE_BASE_CONTAINER_HASH_MAP

#include "mirage_base/define.hpp"
#include "mirage_base/util/hash.hpp"

namespace mirage {

template <HashKeyType Key, std::move_constructible Val>
class HashMapIterator {};

template <HashKeyType Key, std::move_constructible Val>
class HashMapConstIterator {};

template <HashKeyType Key, std::move_constructible Val>
class HashMap {
 public:
  using Iterator = HashMapIterator<Key, Val>;
  using ConstIterator = HashMapConstIterator<Key, Val>;

  HashMap() = default;

  HashMap(const HashMap& other) {
    if constexpr (!std::copy_constructible<Key> ||
                  !std::copy_constructible<Val>) {
      MIRAGE_DCHECK(false);  // This type is supposed to be copyable.
    } else {
      // TODO
    }
  }

  HashMap(HashMap&& other) noexcept {
    // TODO
  }

  class KVPair {
   public:
    KVPair() = delete;
    ~KVPair() = default;

    KVPair(Key&& key, Val&& val) : key_(std::move(key)), val_(std::move(val)) {}

   private:
    Key key_;
    Val val_;
  };

 private:
};

}  // namespace mirage

#endif
