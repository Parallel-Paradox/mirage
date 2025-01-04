#ifndef MIRAGE_BASE_CONTAINER_SINGLY_LINKED_LIST
#define MIRAGE_BASE_CONTAINER_SINGLY_LINKED_LIST

#include <concepts>
#include <initializer_list>
#include <iterator>

#include "mirage_base/define.hpp"

namespace mirage::base {

template <std::move_constructible T>
class SinglyLinkedList {
 public:
  struct Node;

  class Iterator;
  class ConstIterator;

  SinglyLinkedList() = default;

  SinglyLinkedList(SinglyLinkedList&& other) noexcept;
  SinglyLinkedList& operator=(SinglyLinkedList&& other) noexcept;

  SinglyLinkedList(const SinglyLinkedList& other)
    requires std::copy_constructible<T>;
  SinglyLinkedList& operator=(const SinglyLinkedList& other)
    requires std::copy_constructible<T>;

  SinglyLinkedList(std::initializer_list<T> list)
    requires std::copy_constructible<T>;

  ~SinglyLinkedList();

  template <typename... Args>
  void EmplaceHead(Args&&... args);
  void PushHead(const T& val)
    requires std::copy_constructible<T>;

  T RemoveHead();
  void Clear();

  Iterator begin();
  Iterator end();

  ConstIterator begin() const;
  ConstIterator end() const;

 private:
  Node* head_{nullptr};
};

template <std::move_constructible T>
struct SinglyLinkedList<T>::Node {
  T val;
  Node* next{nullptr};

  Node() = delete;
  Node(Node&&) = delete;
  Node(const Node&) = delete;
  ~Node() = default;

  explicit Node(T&& val) : val(std::move(val)) {}
};

template <std::move_constructible T>
class SinglyLinkedList<T>::Iterator {
 public:
  using iterator_concept = std::forward_iterator_tag;
  using iterator_category = std::forward_iterator_tag;
  using iterator_type = Iterator;
  using difference_type = int64_t;
  using value_type = T;
  using pointer = value_type*;
  using reference = value_type&;

  Iterator() = default;
  ~Iterator() = default;

  Iterator(const Iterator& other);

  explicit Iterator(Node* here);

  iterator_type& operator=(const iterator_type& other);
  iterator_type& operator=(std::nullptr_t);
  reference operator*() const;
  pointer operator->() const;
  iterator_type& operator++();
  iterator_type operator++(int);
  bool operator==(const iterator_type& other) const;
  bool operator==(std::nullptr_t) const;
  explicit operator bool() const;

  template <typename... Args>
  void EmplaceAfter(Args&&... args);
  void InsertAfter(const T& val)
    requires std::copy_constructible<T>;
  T RemoveAfter();

 private:
  friend class ConstIterator;

  Node* here_{nullptr};
};

template <std::move_constructible T>
class SinglyLinkedList<T>::ConstIterator {
 public:
  using iterator_concept = std::forward_iterator_tag;
  using iterator_category = std::forward_iterator_tag;
  using iterator_type = ConstIterator;
  using difference_type = int64_t;
  using value_type = const T;
  using pointer = value_type*;
  using reference = value_type&;

  ConstIterator() = default;
  ~ConstIterator() = default;

  ConstIterator(const ConstIterator& other);

  explicit ConstIterator(Node* here);

  // NOLINTNEXTLINE: Convert to const
  ConstIterator(const Iterator& iter);

  iterator_type& operator=(const iterator_type& other);
  iterator_type& operator=(std::nullptr_t);
  reference operator*() const;
  pointer operator->() const;
  iterator_type& operator++();
  iterator_type operator++(int);
  bool operator==(const iterator_type& other) const;
  bool operator==(std::nullptr_t) const;
  explicit operator bool() const;

 private:
  Node* here_{nullptr};
};

template <std::move_constructible T>
SinglyLinkedList<T>::SinglyLinkedList(SinglyLinkedList&& other) noexcept
    : head_(other.head_) {
  other.head_ = nullptr;
}

template <std::move_constructible T>
SinglyLinkedList<T>& SinglyLinkedList<T>::operator=(
    SinglyLinkedList&& other) noexcept {
  if (this != &other) {
    Clear();
    new (this) SinglyLinkedList(std::move(other));
  }
  return *this;
}

template <std::move_constructible T>
SinglyLinkedList<T>::SinglyLinkedList(const SinglyLinkedList& other)
  requires std::copy_constructible<T>
{
  auto iter = other.begin();
  if (iter == other.end()) {
    return;
  }
  Node* ptr = new Node(T(*iter));
  head_ = ptr;
  ++iter;
  while (iter != other.end()) {
    Node* next = new Node(T(*iter));
    ptr->next = next;
    ptr = next;
    ++iter;
  }
}

template <std::move_constructible T>
SinglyLinkedList<T>& SinglyLinkedList<T>::operator=(
    const SinglyLinkedList& other)
  requires std::copy_constructible<T>
{
  if (this != &other) {
    Clear();
    new (this) SinglyLinkedList(other);
  }
  return *this;
}

template <std::move_constructible T>
SinglyLinkedList<T>::SinglyLinkedList(std::initializer_list<T> list)
  requires std::copy_constructible<T>
{
  if (list.size() == 0) {
    return;
  }
  auto iter = list.begin();
  Node* ptr = new Node(T(*iter));
  head_ = ptr;
  ++iter;
  while (iter != list.end()) {
    Node* next = new Node(T(*iter));
    ptr->next = next;
    ptr = next;
    ++iter;
  }
}

template <std::move_constructible T>
SinglyLinkedList<T>::~SinglyLinkedList() {
  Clear();
}

template <std::move_constructible T>
template <typename... Args>
void SinglyLinkedList<T>::EmplaceHead(Args&&... args) {
  Node* new_head = new Node(T(std::forward<Args>(args)...));
  new_head->next = head_;
  head_ = new_head;
}

template <std::move_constructible T>
void SinglyLinkedList<T>::PushHead(const T& val)
  requires std::copy_constructible<T>
{
  EmplaceHead(T(val));
}

template <std::move_constructible T>
T SinglyLinkedList<T>::RemoveHead() {
  MIRAGE_DCHECK(head_ != nullptr);
  T val(std::move(head_->val));
  const Node* head = head_;
  head_ = head_->next;
  delete head;
  return std::move(val);
}

template <std::move_constructible T>
void SinglyLinkedList<T>::Clear() {
  Node* ptr = head_;
  while (ptr != nullptr) {
    Node* next = ptr->next;
    delete ptr;
    ptr = next;
  }
  head_ = nullptr;
}

template <std::move_constructible T>
typename SinglyLinkedList<T>::Iterator SinglyLinkedList<T>::begin() {
  return Iterator(head_);
}

template <std::move_constructible T>
typename SinglyLinkedList<T>::Iterator SinglyLinkedList<T>::end() {
  return Iterator();
}

template <std::move_constructible T>
typename SinglyLinkedList<T>::ConstIterator SinglyLinkedList<T>::begin() const {
  return ConstIterator(head_);
}

template <std::move_constructible T>
typename SinglyLinkedList<T>::ConstIterator SinglyLinkedList<T>::end() const {
  return ConstIterator();
}

template <std::move_constructible T>
SinglyLinkedList<T>::Iterator::Iterator(const Iterator& other)
    : here_(other.here_) {}

template <std::move_constructible T>
SinglyLinkedList<T>::Iterator::Iterator(Node* here) : here_(here) {}

template <std::move_constructible T>
typename SinglyLinkedList<T>::Iterator::iterator_type&
SinglyLinkedList<T>::Iterator::operator=(const iterator_type& other) {
  if (this != &other) {
    here_ = other.here_;
  }
  return *this;
}

template <std::move_constructible T>
typename SinglyLinkedList<T>::Iterator::iterator_type&
SinglyLinkedList<T>::Iterator::operator=(std::nullptr_t) {
  here_ = nullptr;
  return *this;
}

template <std::move_constructible T>
typename SinglyLinkedList<T>::Iterator::reference
SinglyLinkedList<T>::Iterator::operator*() const {
  return here_->val;
}

template <std::move_constructible T>
typename SinglyLinkedList<T>::Iterator::pointer
SinglyLinkedList<T>::Iterator::operator->() const {
  return &(here_->val);
}

template <std::move_constructible T>
typename SinglyLinkedList<T>::Iterator::iterator_type&
SinglyLinkedList<T>::Iterator::operator++() {
  if (here_ != nullptr) {
    here_ = here_->next;
  }
  return *this;
}

template <std::move_constructible T>
typename SinglyLinkedList<T>::Iterator::iterator_type
SinglyLinkedList<T>::Iterator::operator++(int) {
  if (here_ != nullptr) {
    iterator_type temp = *this;
    here_ = here_->next;
    return temp;
  }
  return *this;
}

template <std::move_constructible T>
bool SinglyLinkedList<T>::Iterator::operator==(
    const iterator_type& other) const {
  return here_ == other.here_;
}

template <std::move_constructible T>
bool SinglyLinkedList<T>::Iterator::operator==(std::nullptr_t) const {
  return here_ == nullptr;
}

template <std::move_constructible T>
SinglyLinkedList<T>::Iterator::operator bool() const {
  return here_ != nullptr;
}

template <std::move_constructible T>
template <typename... Args>
void SinglyLinkedList<T>::Iterator::EmplaceAfter(Args&&... args) {
  Node* new_node = new Node(T(std::forward<Args>(args)...));
  new_node->next = here_->next;
  here_->next = new_node;
}

template <std::move_constructible T>
void SinglyLinkedList<T>::Iterator::InsertAfter(const T& val)
  requires std::copy_constructible<T>
{
  EmplaceAfter(T(val));
}

template <std::move_constructible T>
T SinglyLinkedList<T>::Iterator::RemoveAfter() {
  MIRAGE_DCHECK(here_ != nullptr && here_->next != nullptr);
  T val(std::move(here_->next->val));
  const Node* next = here_->next;
  here_->next = nullptr;
  delete next;
  return std::move(val);
}

template <std::move_constructible T>
SinglyLinkedList<T>::ConstIterator::ConstIterator(const ConstIterator& other)
    : here_(other.here_) {}

template <std::move_constructible T>
SinglyLinkedList<T>::ConstIterator::ConstIterator(Node* here) : here_(here) {}

template <std::move_constructible T>
SinglyLinkedList<T>::ConstIterator::ConstIterator(const Iterator& iter)
    : here_(iter.here_) {}

template <std::move_constructible T>
typename SinglyLinkedList<T>::ConstIterator::iterator_type&
SinglyLinkedList<T>::ConstIterator::operator=(const iterator_type& other) {
  if (this != &other) {
    here_ = other.here_;
  }
  return *this;
}

template <std::move_constructible T>
typename SinglyLinkedList<T>::ConstIterator::iterator_type&
SinglyLinkedList<T>::ConstIterator::operator=(std::nullptr_t) {
  here_ = nullptr;
  return *this;
}

template <std::move_constructible T>
typename SinglyLinkedList<T>::ConstIterator::reference
SinglyLinkedList<T>::ConstIterator::operator*() const {
  return here_->val;
}

template <std::move_constructible T>
typename SinglyLinkedList<T>::ConstIterator::pointer
SinglyLinkedList<T>::ConstIterator::operator->() const {
  return &here_->val;
}

template <std::move_constructible T>
typename SinglyLinkedList<T>::ConstIterator::iterator_type&
SinglyLinkedList<T>::ConstIterator::operator++() {
  if (here_ != nullptr) {
    here_ = here_->next;
  }
  return *this;
}

template <std::move_constructible T>
typename SinglyLinkedList<T>::ConstIterator::iterator_type
SinglyLinkedList<T>::ConstIterator::operator++(int) {
  if (here_ != nullptr) {
    iterator_type temp = *this;
    here_ = here_->next;
    return temp;
  }
  return *this;
}

template <std::move_constructible T>
bool SinglyLinkedList<T>::ConstIterator::operator==(
    const iterator_type& other) const {
  return here_ == other.here_;
}

template <std::move_constructible T>
bool SinglyLinkedList<T>::ConstIterator::operator==(std::nullptr_t) const {
  return here_ == nullptr;
}

template <std::move_constructible T>
SinglyLinkedList<T>::ConstIterator::operator bool() const {
  return here_ != nullptr;
}

}  // namespace mirage::base

#endif  // MIRAGE_BASE_CONTAINER_SINGLY_LINKED_LIST
