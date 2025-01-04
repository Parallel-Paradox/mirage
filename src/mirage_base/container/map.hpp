#ifndef MIRAGE_BASE_CONTAINER_MAP
#define MIRAGE_BASE_CONTAINER_MAP

#include "mirage_base/container/set.hpp"
#include "mirage_base/util/key_val.hpp"

namespace mirage::base {

template <RBTreeNodeType Key, std::move_constructible Val,
          bool IS_DUPLICATE_ALLOWED>
class MapBase {
 public:
  struct Entry;

  using InsertResult =
      std::conditional_t<IS_DUPLICATE_ALLOWED, void, Optional<Val>>;

  MapBase() = default;
  ~MapBase();

  InsertResult Insert(Key&& key, Val&& val);
  Optional<Val> Remove(const Key& key);
  void Clear();

 private:
  RBTree<Entry, IS_DUPLICATE_ALLOWED> entry_set_;
};

template <RBTreeNodeType Key, std::move_constructible Val,
          bool IS_DUPLICATE_ALLOWED>
struct MapBase<Key, Val, IS_DUPLICATE_ALLOWED>::Entry : KeyVal<Key, Val> {
  Entry(Key&& key, Val&& val);

  std::strong_ordering operator<=>(const Entry& other) const;
  std::strong_ordering operator<=>(const Key& other_key) const;
  bool operator==(const Entry& other) const;
  bool operator==(const Key& other_key) const;
};

template <RBTreeNodeType Key, std::move_constructible Val, bool D>
MapBase<Key, Val, D>::MapBase::~MapBase() {
  Clear();
}

template <RBTreeNodeType Key, std::move_constructible Val, bool D>
typename MapBase<Key, Val, D>::MapBase::InsertResult
MapBase<Key, Val, D>::MapBase::Insert(Key&& key, Val&& val) {
  Entry entry(std::move(key), std::move(val));
  if constexpr (D) {
    entry_set_.Insert(std::move(entry));
    return;
  } else {
    auto old_entry = entry_set_.Insert(std::move(entry));
    if (!old_entry.IsValid()) {
      return Optional<Val>::None();
    }
    return Optional<Val>(std::move(old_entry.Unwrap().val));
  }
}

template <RBTreeNodeType Key, std::move_constructible Val, bool D>
Optional<Val> MapBase<Key, Val, D>::MapBase::Remove(const Key& key) {
  auto old_entry = entry_set_.Remove(entry_set_.TryFind(key));
  if (!old_entry.IsValid()) {
    return Optional<Val>::None();
  }
  return Optional<Val>(std::move(old_entry.Unwrap().val));
}

template <RBTreeNodeType Key, std::move_constructible Val, bool D>
void MapBase<Key, Val, D>::MapBase::Clear() {
  entry_set_.Clear();
}

template <RBTreeNodeType Key, std::move_constructible Val, bool D>
MapBase<Key, Val, D>::Entry::Entry(Key&& key, Val&& val)
    : KeyVal<Key, Val>(std::move(key), std::move(val)) {}

template <RBTreeNodeType Key, std::move_constructible Val, bool D>
std::strong_ordering MapBase<Key, Val, D>::Entry::operator<=>(
    const Entry& other) const {
  return this->key <=> other.key;
}

template <RBTreeNodeType Key, std::move_constructible Val, bool D>
std::strong_ordering MapBase<Key, Val, D>::Entry::operator<=>(
    const Key& other_key) const {
  return this->key <=> other_key;
}

template <RBTreeNodeType Key, std::move_constructible Val, bool D>
bool MapBase<Key, Val, D>::Entry::operator==(const Entry& other) const {
  return this->key == other.key;
}

template <RBTreeNodeType Key, std::move_constructible Val, bool D>
bool MapBase<Key, Val, D>::Entry::operator==(const Key& other_key) const {
  return this->key == other_key;
}

template <RBTreeNodeType Key, std::move_constructible Val>
using MultiMap = MapBase<Key, Val, true>;

template <RBTreeNodeType Key, std::move_constructible Val>
using Map = MapBase<Key, Val, false>;

}  // namespace mirage::base

#endif  // MIRAGE_BASE_CONTAINER_MAP
