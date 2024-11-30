#ifndef MIRAGE_BASE_CONTAINER_HASH_MAP
#define MIRAGE_BASE_CONTAINER_HASH_MAP

#include <initializer_list>
#include <utility>

#include "mirage_base/container/array.hpp"
#include "mirage_base/container/singly_linked_list.hpp"
#include "mirage_base/define.hpp"
#include "mirage_base/util/aligned_memory.hpp"
#include "mirage_base/util/hash.hpp"
#include "mirage_base/util/optional.hpp"

namespace mirage {

template <HashKeyType Key, std::move_constructible Val>
class HashMapIterator {
  // TODO
};

template <HashKeyType Key, std::move_constructible Val>
class HashMapConstIterator {
  // TODO
};

template <HashKeyType Key, std::move_constructible Val>
class HashMap {
 public:
  using Iterator = HashMapIterator<Key, Val>;
  using ConstIterator = HashMapConstIterator<Key, Val>;

  HashMap(Hash<Key> hasher = Hash<Key>()) : hasher_(std::move(hasher)) {}

  HashMap(const HashMap& other) : hasher_(other.hasher_) {
    if constexpr (!std::copy_constructible<Key> ||
                  !std::copy_constructible<Val>) {
      MIRAGE_DCHECK(false);  // This type is supposed to be copyable.
    } else {
      buckets_ = other.buckets_;
      max_bucket_size_ = other.max_bucket_size_;
      size_ = other.size_;
    }
  }

  HashMap& operator=(const HashMap& other) {
    if constexpr (!std::copy_constructible<Key> ||
                  !std::copy_constructible<Val>) {
      MIRAGE_DCHECK(false);  // This type is supposed to be copyable.
    } else {
      if (this != &other) {
        Clear();
        new (this) HashMap(other);
      }
    }
    return *this;
  }

  HashMap(HashMap&& other) noexcept : hasher_(std::move(other.hasher_)) {
    buckets_ = std::move(other.buckets_);
    max_bucket_size_ = other.max_bucket_size_;
    size_ = other.size_;

    other.Clear();
  }

  HashMap& operator=(HashMap&& other) noexcept {
    if (this != &other) {
      Clear();
      new (this) HashMap(std::move(other));
    }
    return *this;
  }

  class KVPair {
   public:
    KVPair() = delete;
    ~KVPair() = default;

    KVPair(const KVPair& other) {
      if constexpr (!std::copy_constructible<Key> ||
                    !std::copy_constructible<Val>) {
        MIRAGE_DCHECK(false);  // This type is supposed to be copyable.
      } else {
        new (key_.GetPtr()) Key(other.key_.GetRef());
        new (val_.GetPtr()) Val(other.val_.GetRef());
      }
    }

    KVPair(KVPair&& other) noexcept {
      new (key_.GetPtr()) Key(std::move(other.key_.GetRef()));
      new (val_.GetPtr()) Val(std::move(other.val_.GetRef()));
    }

    KVPair(Key&& key, Val&& val) {
      new (key_.GetPtr()) Key(std::move(key));
      new (val_.GetPtr()) Val(std::move(val));
    }

    const Key& GetConstKey() const { return key_.GetConstRef(); }

    const Val& GetConstVal() const { return val_.GetConstRef(); }

    Val& GetVal() { return val_.GetRef(); }

   private:
    AlignedMemory<Key> key_;
    AlignedMemory<Val> val_;
  };

  HashMap(std::initializer_list<KVPair> list, Hash<Key> hasher = Hash<Key>())
      : hasher_(std::move(hasher)) {
    if constexpr (!std::copy_constructible<Key> ||
                  !std::copy_constructible<Val>) {
      MIRAGE_DCHECK(false);  // This type is supposed to be copyable.
    } else {
      for (const KVPair& pair : list) {
        Insert(Key(pair.GetConstKey()), Val(pair.GetConstVal()));
      }
    }
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
    if (IsEmpty()) {
      return nullptr;
    }

    size_t hash = hasher_(key);
    size_t index = hash % buckets_.GetSize();
    auto& bucket = buckets_[index];
    for (auto iter = bucket.list.begin(); iter != bucket.list.end(); ++iter) {
      if (iter->GetConstKey() == key) {
        return &(iter->GetVal());
      }
    }
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
    SinglyLinkedList<KVPair> list;
    uint32_t size{0};

    Bucket() = default;
    ~Bucket() = default;

    Bucket(const Bucket& other) {
      if constexpr (!std::copy_constructible<Key> ||
                    !std::copy_constructible<Val>) {
        MIRAGE_DCHECK(false);  // This type is supposed to be copyable.
      } else {
        for (auto iter = other.list.begin(); iter != other.list.end(); ++iter) {
          list.EmplaceHead(Key(iter->GetConstKey()), Val(iter->GetConstVal()));
        }
        size = other.size;
      }
    }

    Bucket(Bucket&& other) noexcept
        : list(std::move(other.list)), size(other.size) {
      other.size = 0;
    }
  };

  Hash<Key> hasher_;
  Array<Bucket> buckets_;
  uint32_t max_bucket_size_{8};
  size_t size_{0};
};

}  // namespace mirage

#endif
