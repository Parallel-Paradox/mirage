#ifndef MIRAGE_BASE_CONTAINER_HASH_MAP
#define MIRAGE_BASE_CONTAINER_HASH_MAP

#include <initializer_list>

#include "mirage_base/container/array.hpp"
#include "mirage_base/container/singly_linked_list.hpp"
#include "mirage_base/define.hpp"
#include "mirage_base/util/hash.hpp"
#include "mirage_base/util/optional.hpp"

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

  HashMap(Hash<Key> hasher = Hash<Key>()) : hasher_(std::move(hasher)) {}

  HashMap(const HashMap& other) {
    if constexpr (!std::copy_constructible<Key> ||
                  !std::copy_constructible<Val> ||
                  !std::copy_constructible<Hash<Key>>) {
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

    const Key& GetKey() const { return key_; }

    const Val& GetVal() const { return val_; }

    Val& GetVal() { return val_; }

   private:
    Key key_;
    Val val_;
  };

  HashMap(std::initializer_list<KVPair> list, Hash<Key> hasher = Hash<Key>()) {
    // TODO
  }

  ~HashMap() noexcept { Clear(); }

  Optional<Val> Insert(Key&& key, Val&& val) {
    Val* old = TryFind(key);
    if (old != nullptr) {
      auto ret = Optional<Val>::New(std::move(*old));
      new (old) Val(std::move(val));
      return ret;
    }

    // TODO
    return Optional<Val>::None();
  }

  Optional<Val> Remove(const Key& key) {
    // TODO
    return Optional<Val>::None();
  }

  Val* TryFind(const Key& key) const {
    size_t hash = hasher_(key);
    // TODO
    return nullptr;
  }

  Val& Find(const Key& key) const { return *TryFind(key); }

  Val& operator[](const Key& key) const { return *TryFind(key); }

  void Clear() {
    buckets_.Clear();
    size_ = 0;
  }

  size_t GetSize() const { return size_; }

  bool IsEmpty() const { return size_ == 0; }

 private:
  struct Bucket {
    SinglyLinkedList<KVPair> list_;
    uint32_t size_{0};
  };

  Hash<Key> hasher_;
  Array<Bucket> buckets_;
  uint32_t max_bucket_size_{8};
  size_t size_{0};
};

}  // namespace mirage

#endif
