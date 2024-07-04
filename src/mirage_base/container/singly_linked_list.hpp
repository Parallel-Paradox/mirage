#ifndef MIRAGE_BASE_CONTAINER_SINGLY_LINKED_LIST
#define MIRAGE_BASE_CONTAINER_SINGLY_LINKED_LIST

#include <concepts>
#include <initializer_list>
#include <iterator>

#include "mirage_base/define.hpp"

namespace mirage {

template <std::move_constructible T>
struct SinglyLinkedListNode {
  T val_;
  SinglyLinkedListNode* next_{nullptr};

  SinglyLinkedListNode() = delete;
  SinglyLinkedListNode(SinglyLinkedListNode&&) = delete;
  SinglyLinkedListNode(const SinglyLinkedListNode&) = delete;
  ~SinglyLinkedListNode() = default;

  explicit SinglyLinkedListNode(T&& val) : val_(std::move(val)) {}
};

template <std::move_constructible T>
class SinglyLinkedListConstIterator;

template <std::move_constructible T>
class SinglyLinkedListIterator {
 public:
  using iterator_concept = std::forward_iterator_tag;
  using iterator_category = std::forward_iterator_tag;
  using iterator_type = SinglyLinkedListIterator;
  using difference_type = int64_t;
  using value_type = T;
  using pointer = value_type*;
  using reference = value_type&;

  using Node = SinglyLinkedListNode<T>;

  SinglyLinkedListIterator() = default;
  ~SinglyLinkedListIterator() = default;

  SinglyLinkedListIterator(const SinglyLinkedListIterator& other)
      : here_(other.here_) {}

  explicit SinglyLinkedListIterator(Node* here) : here_(here) {}

  iterator_type& operator=(const iterator_type& other) {
    if (this != &other) {
      here_ = other.here_;
    }
    return *this;
  }

  iterator_type& operator=(std::nullptr_t) {
    here_ = nullptr;
    return *this;
  }

  reference operator*() const { return here_->val_; }

  pointer operator->() const { return &(here_->val_); }

  iterator_type& operator++() {
    if (here_ != nullptr) {
      here_ = here_->next_;
    }
    return *this;
  }

  iterator_type operator++(int) {
    if (here_ != nullptr) {
      iterator_type temp = *this;
      here_ = here_->next_;
      return temp;
    } else {
      return *this;
    }
  }

  bool operator==(const iterator_type& other) const {
    return here_ == other.here_;
  }

  template <typename... Args>
  void EmplaceAfter(Args&&... args) {
    Node* new_node = new Node(T(std::forward<Args>(args)...));
    new_node->next_ = here_->next_;
    here_->next_ = new_node;
  }

  void InsertAfter(const T& val) {
    if constexpr (!std::copy_constructible<T>) {
      MIRAGE_DCHECK(false);  // This type is supposed to be copyable.
    } else {
      EmplaceAfter(T(val));
    }
  }

  T RemoveAfter() {
    MIRAGE_DCHECK(here_ != nullptr && here_->next_ != nullptr);
    T val(std::move(here_->next_->val_));
    Node* next = here_->next_;
    here_->next_ = nullptr;
    delete next;
    return std::move(val);
  }

 private:
  friend class SinglyLinkedListConstIterator<T>;

  Node* here_{nullptr};
};

template <std::move_constructible T>
class SinglyLinkedListConstIterator {
 public:
  using iterator_concept = std::forward_iterator_tag;
  using iterator_category = std::forward_iterator_tag;
  using iterator_type = SinglyLinkedListConstIterator;
  using difference_type = int64_t;
  using value_type = const T;
  using pointer = value_type*;
  using reference = value_type&;

  using Node = SinglyLinkedListNode<T>;

  SinglyLinkedListConstIterator() = default;
  ~SinglyLinkedListConstIterator() = default;

  SinglyLinkedListConstIterator(const SinglyLinkedListConstIterator& other)
      : here_(other.here_) {}

  explicit SinglyLinkedListConstIterator(Node* here) : here_(here) {}

  explicit SinglyLinkedListConstIterator(
      const SinglyLinkedListIterator<T>& iter)
      : here_(iter.here_) {}

  iterator_type& operator=(const iterator_type& other) {
    if (this != &other) {
      here_ = other.here_;
    }
    return *this;
  }

  iterator_type& operator=(std::nullptr_t) {
    here_ = nullptr;
    return *this;
  }

  reference operator*() const { return here_->val_; }

  pointer operator->() const { return &(here_->val_); }

  iterator_type& operator++() {
    if (here_ != nullptr) {
      here_ = here_->next_;
    }
    return *this;
  }

  iterator_type operator++(int) {
    if (here_ != nullptr) {
      iterator_type temp = *this;
      here_ = here_->next_;
      return temp;
    } else {
      return *this;
    }
  }

  bool operator==(const iterator_type& other) const {
    return here_ == other.here_;
  }

 private:
  Node* here_{nullptr};
};

template <std::move_constructible T>
class SinglyLinkedList {
 public:
  using Node = SinglyLinkedListNode<T>;
  using Iterator = SinglyLinkedListIterator<T>;
  using ConstIterator = SinglyLinkedListConstIterator<T>;

  SinglyLinkedList() = default;

  SinglyLinkedList(SinglyLinkedList&& other) noexcept : head_(other.head_) {
    other.head_ = nullptr;
  }

  SinglyLinkedList(const SinglyLinkedList& other) {
    if constexpr (!std::copy_constructible<T>) {
      MIRAGE_DCHECK(false);  // This type is supposed to be copyable.
    } else {
      auto iter = other.begin();
      if (iter == other.end()) {
        return;
      }
      Node* ptr = new Node(T(*iter));
      head_ = ptr;
      ++iter;
      while (iter != other.end()) {
        Node* next = new Node(T(*iter));
        ptr->next_ = next;
        ptr = next;
        ++iter;
      }
    }
  }

  SinglyLinkedList(std::initializer_list<T> list) {
    if constexpr (!std::copy_constructible<T>) {
      MIRAGE_DCHECK(false);  // This type is supposed to be copyable.
    } else {
      if (list.size() == 0) {
        return;
      }
      auto iter = list.begin();
      Node* ptr = new Node(T(*iter));
      head_ = ptr;
      ++iter;
      while (iter != list.end()) {
        Node* next = new Node(T(*iter));
        ptr->next_ = next;
        ptr = next;
        ++iter;
      }
    }
  }

  ~SinglyLinkedList() {
    Node* ptr = head_;
    while (ptr != nullptr) {
      Node* next = ptr->next_;
      delete ptr;
      ptr = next;
    }
  }

  template <typename... Args>
  void EmplaceHead(Args&&... args) {
    Node* new_head = new Node(T(std::forward<Args>(args)...));
    new_head->next_ = head_;
    head_ = new_head;
  }

  void PushHead(const T& val) {
    if constexpr (!std::copy_constructible<T>) {
      MIRAGE_DCHECK(false);  // This type is supposed to be copyable.
    } else {
      EmplaceHead(T(val));
    }
  }

  T RemoveHead() {
    MIRAGE_DCHECK(head_ != nullptr);
    T val(std::move(head_->val_));
    Node* head = head_;
    head_ = head_->next_;
    delete head;
    return std::move(val);
  }

  Iterator begin() { return Iterator(head_); }

  Iterator end() { return Iterator(); }

  ConstIterator begin() const { return ConstIterator(head_); }

  ConstIterator end() const { return ConstIterator(); }

 private:
  Node* head_{nullptr};
};

}  // namespace mirage

#endif  // MIRAGE_BASE_CONTAINER_SINGLY_LINKED_LIST
