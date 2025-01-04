#ifndef MIRAGE_BASE_CONTAINER_SET
#define MIRAGE_BASE_CONTAINER_SET

#include <concepts>

#include "mirage_base/container/array.hpp"
#include "mirage_base/util/aligned_memory.hpp"
#include "mirage_base/util/optional.hpp"

namespace mirage::base {

template <typename T>
concept RBTreeNodeType = std::move_constructible<T> && std::totally_ordered<T>;

template <RBTreeNodeType T, bool IS_DUPLICATE_ALLOWED = true>
class RBTree {
 public:
  struct Node;
  class ConstIterator;

  using InsertResult =
      std::conditional_t<IS_DUPLICATE_ALLOWED, void, Optional<T>>;

  RBTree();
  ~RBTree();

  RBTree(std::initializer_list<T> list)
    requires std::copy_constructible<T>;

  InsertResult Insert(T&& val);
  Optional<T> Remove(const T& val);
  Optional<T> Remove(const ConstIterator& target);

  template <typename T1>
  ConstIterator TryFind(const T1& val) const
    requires requires(const T1& val, const T& entry) {
      { val == entry } -> std::convertible_to<bool>;
      { val < entry } -> std::convertible_to<bool>;
    };
  size_t Count(const T& val) const;

  void Clear();
  [[nodiscard]] bool IsEmpty() const;
  [[nodiscard]] size_t GetSize() const;

  ConstIterator begin() const;
  ConstIterator end() const;

 private:
  void RotateLeft(Node& node);
  void RotateRight(Node& node);

  constexpr InsertResult None();

  Node* root_;
  size_t size_;
};

template <RBTreeNodeType T, bool D>
struct RBTree<T, D>::Node {
 private:
  Node() : parent(this), left(this), right(this), color(BLACK) {}

  static Node* Null() {
    static Node sentinel_null_node;
    return &sentinel_null_node;
  }

  friend class RBTree;
  friend class ConstIterator;

 public:
  enum Color { RED, BLACK };

  AlignedMemory<T> val;
  Node* parent{Null()};
  Node* left{Null()};
  Node* right{Null()};
  Color color{RED};

  Node(Node&&) = delete;
  Node(const Node&) = delete;

  ~Node() {
    // Only Null() will create uninitialized node, which is static
    if (this != Null()) [[likely]] {
      val.GetPtr()->~T();
    }
    parent = nullptr;
    left = nullptr;
    right = nullptr;
  }

  explicit Node(T&& val) : val(std::move(val)) {}
};

template <RBTreeNodeType T, bool D>
class RBTree<T, D>::ConstIterator {
 public:
  using iterator_concept = std::bidirectional_iterator_tag;
  using iterator_category = std::bidirectional_iterator_tag;
  using iterator_type = ConstIterator;
  using difference_type = int64_t;
  using value_type = const T;
  using pointer = value_type*;
  using reference = value_type&;

  ConstIterator() = default;
  ~ConstIterator() = default;

  ConstIterator(const ConstIterator& other);

  explicit ConstIterator(Node& here);

  iterator_type& operator=(const iterator_type& other);
  iterator_type& operator=(std::nullptr_t);
  reference operator*() const;
  pointer operator->() const;
  iterator_type& operator++();
  iterator_type operator++(int);
  iterator_type& operator--();
  iterator_type operator--(int);
  bool operator==(const iterator_type& other) const;

 private:
  friend class RBTree;

  Node* here_{Node::Null()};
};

template <RBTreeNodeType T, bool D>
RBTree<T, D>::RBTree::RBTree() : root_(Node::Null()), size_(0) {}

template <RBTreeNodeType T, bool D>
RBTree<T, D>::RBTree::~RBTree() {
  Clear();
}

template <RBTreeNodeType T, bool D>
RBTree<T, D>::RBTree::RBTree(std::initializer_list<T> list)
  requires std::copy_constructible<T>
    : root_(Node::Null()), size_(0) {
  for (const T& val : list) {
    Insert(T(val));
  }
}

template <RBTreeNodeType T, bool D>
typename RBTree<T, D>::RBTree::InsertResult RBTree<T, D>::RBTree::Insert(
    T&& val) {
  // Find insert place
  Node* parent = nullptr;
  Node* iter = root_;
  while (iter != Node::Null()) {
    parent = iter;
    if (val < iter->val.GetConstRef()) {
      iter = iter->left;
    } else if constexpr (D) {  // NOLINT: comptime branch
      iter = iter->right;
    } else if (val == iter->val.GetConstRef()) {
      Optional<T> rv(std::move(iter->val.GetRef()));
      new (iter->val.GetPtr()) T(std::move(val));
      return rv;
    } else {
      iter = iter->right;
    }
  }

  // Insert root node
  ++size_;
  iter = new Node(std::move(val));
  if (parent == nullptr) {
    iter->color = Node::BLACK;
    root_ = iter;
    return None();
  }

  if (iter->val.GetConstRef() < parent->val.GetConstRef()) {
    parent->left = iter;
  } else {
    parent->right = iter;
  }
  iter->parent = parent;

  // Fix color
  if (parent->color == Node::BLACK) {
    return None();
  }
  while (iter->parent->color == Node::RED) {
    parent = iter->parent;
    Node* grand = parent->parent;
    if (Node* uncle = parent == grand->left ? grand->right : grand->left;
        uncle->color == Node::RED) {
      parent->color = Node::BLACK;
      uncle->color = Node::BLACK;
      grand->color = Node::RED;
      iter = grand;
      continue;
    }
    if (parent == grand->left) {
      if (iter == parent->right) {
        RotateLeft(*parent);
        parent = iter;
      }
      RotateRight(*grand);
    } else {
      if (iter == parent->left) {
        RotateRight(*parent);
        parent = iter;
      }
      RotateLeft(*grand);
    }
    parent->color = Node::BLACK;
    grand->color = Node::RED;
    break;
  }
  root_->color = Node::BLACK;
  return None();
}

template <RBTreeNodeType T, bool D>
Optional<T> RBTree<T, D>::RBTree::Remove(const T& val) {
  return Remove(TryFind(val));
}

template <RBTreeNodeType T, bool D>
Optional<T> RBTree<T, D>::RBTree::Remove(const ConstIterator& target) {
  // Find remove place
  Node* val_ptr = const_cast<Node*>(target.here_);
  if (val_ptr == Node::Null()) {
    return Optional<T>::None();
  }
  --size_;
  Optional<T> rv(std::move(val_ptr->val.GetRef()));

  bool left_exists = val_ptr->left != Node::Null();
  bool right_exists = val_ptr->right != Node::Null();

  if (left_exists && right_exists) {
    // Reduce the problem with successor
    Node* successor = val_ptr->right;
    while (successor->left != Node::Null()) {
      successor = successor->left;
    }
    new (val_ptr->val.GetPtr()) T(std::move(successor->val.GetRef()));
    val_ptr = successor;
    left_exists = false;
    right_exists = val_ptr->right != Node::Null();
  }

  Node* parent = val_ptr->parent;
  if (!left_exists && !right_exists && val_ptr->color == Node::RED) {
    if (val_ptr == parent->left) {
      parent->left = Node::Null();
    } else {
      parent->right = Node::Null();
    }
    delete val_ptr;
    return rv;
  }

  if (left_exists) {  // && !right_exists
    MIRAGE_DCHECK(val_ptr->color == Node::BLACK);
    if (val_ptr == parent->left) {
      parent->left = val_ptr->left;
    } else {
      parent->right = val_ptr->left;
    }
    val_ptr->left->parent = parent;
    val_ptr->left->color = Node::BLACK;
    delete val_ptr;
    return rv;
  }
  if (right_exists) {  // && !left_exists
    MIRAGE_DCHECK(val_ptr->color == Node::BLACK);
    if (val_ptr == parent->left) {
      parent->left = val_ptr->right;
    } else {
      parent->right = val_ptr->right;
    }
    val_ptr->right->parent = parent;
    val_ptr->right->color = Node::BLACK;
    delete val_ptr;
    return rv;
  }

  // !left_exists && !right_exists && iter->color == Node::BLACK
  if (val_ptr == root_) {
    root_ = Node::Null();
    delete val_ptr;
    return rv;
  }

  Node* iter = val_ptr;
  while (iter != root_) {
    parent = iter->parent;
    if (iter == parent->left) {
      Node* brother = parent->right;
      if (brother->color == Node::RED) {
        MIRAGE_DCHECK(parent->color == Node::BLACK);
        RotateLeft(*parent);
        brother->color = Node::BLACK;
        parent->color = Node::RED;
        parent->left = Node::Null();
        break;
      }
      if (brother->left->color == Node::RED) {
        RotateRight(*brother);
        brother = brother->parent;
      }
      if (brother->right->color == Node::RED) {
        RotateLeft(*parent);
        brother->right->color = Node::BLACK;
        parent->left = Node::Null();
        break;
      }
      if (parent->color == Node::RED) {
        parent->color = Node::BLACK;
        brother->color = Node::RED;
        parent->left = Node::Null();
        break;
      }
      brother->color = Node::RED;
      iter = parent;
    } else {
      Node* brother = parent->left;
      if (brother->color == Node::RED) {
        MIRAGE_DCHECK(parent->color == Node::BLACK);
        RotateRight(*parent);
        brother->color = Node::BLACK;
        parent->color = Node::RED;
        parent->right = Node::Null();
        break;
      }
      if (brother->right->color == Node::RED) {
        RotateRight(*brother);
        brother = brother->parent;
      }
      if (brother->left->color == Node::RED) {
        RotateRight(*parent);
        brother->left->color = Node::BLACK;
        parent->right = Node::Null();
        break;
      }
      if (parent->color == Node::RED) {
        parent->color = Node::BLACK;
        brother->color = Node::RED;
        parent->right = Node::Null();
        break;
      }
      brother->color = Node::RED;
      iter = parent;
    }
  }
  delete val_ptr;
  return rv;
}

template <RBTreeNodeType T, bool D>
template <typename T1>
typename RBTree<T, D>::RBTree::ConstIterator RBTree<T, D>::RBTree::TryFind(
    const T1& val) const
  requires requires(const T1& val, const T& entry) {
    { val == entry } -> std::convertible_to<bool>;
    { val < entry } -> std::convertible_to<bool>;
  }
{
  Node* iter = root_;
  while (iter != Node::Null()) {
    if (val == iter->val.GetConstRef()) {
      return ConstIterator(*iter);
    }
    if (val < iter->val.GetConstRef()) {
      iter = iter->left;
    } else {
      iter = iter->right;
    }
  }
  return end();
}

template <RBTreeNodeType T, bool D>
size_t RBTree<T, D>::RBTree::Count(const T& val) const {
  if constexpr (D) {
    ConstIterator val_iter = TryFind(val);
    if (val_iter == end()) {
      return 0;
    }
    ConstIterator iter = val_iter;
    size_t rv = 0;
    while (iter != end() && *iter == val) {
      ++rv;
      ++iter;
    }
    iter = val_iter;
    --iter;
    while (iter != end() && *iter == val) {
      ++rv;
      --iter;
    }
    return rv;
  } else {
    if (TryFind(val) != end()) {
      return 1;
    }
    return 0;
  }
}

template <RBTreeNodeType T, bool D>
void RBTree<T, D>::RBTree::Clear() {
  if (root_ == Node::Null())
    return;

  Array<Node*> stack;
  stack.Push(root_);
  while (!stack.IsEmpty()) {
    Node* node = stack.Pop();
    if (node->left != Node::Null())
      stack.Push(node->left);
    if (node->right != Node::Null())
      stack.Push(node->right);
    delete node;
  }
  root_ = Node::Null();
  size_ = 0;
}

template <RBTreeNodeType T, bool D>
[[nodiscard]] bool RBTree<T, D>::RBTree::IsEmpty() const {
  return size_ == 0;
}

template <RBTreeNodeType T, bool D>
[[nodiscard]] size_t RBTree<T, D>::RBTree::GetSize() const {
  return size_;
}

template <RBTreeNodeType T, bool D>
typename RBTree<T, D>::RBTree::ConstIterator RBTree<T, D>::RBTree::begin()
    const {
  Node* iter = root_;
  while (iter->left != Node::Null()) {
    iter = iter->left;
  }
  return ConstIterator(*iter);
}

template <RBTreeNodeType T, bool D>
typename RBTree<T, D>::RBTree::ConstIterator RBTree<T, D>::RBTree::end() const {
  return ConstIterator();
}

template <RBTreeNodeType T, bool D>
void RBTree<T, D>::RBTree::RotateLeft(Node& node) {
  Node* r = node.right;
  MIRAGE_DCHECK(r != Node::Null());

  node.right = r->left;
  if (r->left != Node::Null()) {
    r->left->parent = &node;
  }

  r->parent = node.parent;
  if (&node == root_) {
    root_ = r;
  } else if (&node == node.parent->left) {
    node.parent->left = r;
  } else {
    node.parent->right = r;
  }
  r->left = &node;
  node.parent = r;
}

template <RBTreeNodeType T, bool D>
void RBTree<T, D>::RBTree::RotateRight(Node& node) {
  Node* l = node.left;
  MIRAGE_DCHECK(l != Node::Null());

  node.left = l->right;
  if (l->right != Node::Null()) {
    l->right->parent = &node;
  }

  l->parent = node.parent;
  if (&node == root_) {
    root_ = l;
  } else if (&node == node.parent->right) {
    node.parent->right = l;
  } else {
    node.parent->left = l;
  }
  l->right = &node;
  node.parent = l;
}

template <RBTreeNodeType T, bool D>
constexpr typename RBTree<T, D>::InsertResult RBTree<T, D>::None() {
  if constexpr (D) {
    return;
  } else {
    return Optional<T>::None();
  }
}

template <RBTreeNodeType T, bool D>
RBTree<T, D>::ConstIterator::ConstIterator(const ConstIterator& other)
    : here_(other.here_) {}

template <RBTreeNodeType T, bool D>
RBTree<T, D>::ConstIterator::ConstIterator(Node& here) : here_(&here) {}

template <RBTreeNodeType T, bool D>
typename RBTree<T, D>::ConstIterator::iterator_type&
RBTree<T, D>::ConstIterator::operator=(const iterator_type& other) {
  if (this != &other) {
    here_ = other.here_;
  }
  return *this;
}

template <RBTreeNodeType T, bool D>
typename RBTree<T, D>::ConstIterator::iterator_type&
RBTree<T, D>::ConstIterator::operator=(std::nullptr_t) {
  here_ = Node::Null();
  return *this;
}

template <RBTreeNodeType T, bool D>
typename RBTree<T, D>::ConstIterator::reference
RBTree<T, D>::ConstIterator::operator*() const {
  return here_->val.GetConstRef();
}

template <RBTreeNodeType T, bool D>
typename RBTree<T, D>::ConstIterator::pointer
RBTree<T, D>::ConstIterator::operator->() const {
  return &here_->val.GetConstPtr();
}

template <RBTreeNodeType T, bool D>
typename RBTree<T, D>::ConstIterator::iterator_type&
RBTree<T, D>::ConstIterator::operator++() {
  if (here_ == Node::Null()) {
    return *this;
  }
  if (here_->right != Node::Null()) {
    here_ = here_->right;
    while (here_->left != Node::Null()) {
      here_ = here_->left;
    }
  } else {
    Node* parent = here_->parent;
    while (parent != Node::Null() && here_ == parent->right) {
      here_ = parent;
      parent = here_->parent;
    }
    here_ = parent;
  }
  return *this;
}

template <RBTreeNodeType T, bool D>
typename RBTree<T, D>::ConstIterator::iterator_type
RBTree<T, D>::ConstIterator::operator++(int) {
  iterator_type temp = *this;
  this->operator++();
  return temp;
}

template <RBTreeNodeType T, bool D>
typename RBTree<T, D>::ConstIterator::iterator_type&
RBTree<T, D>::ConstIterator::operator--() {
  if (here_ == Node::Null()) {
    return *this;
  }
  if (here_->left != Node::Null()) {
    here_ = here_->left;
    while (here_->right != Node::Null()) {
      here_ = here_->right;
    }
  } else {
    Node* parent = here_->parent;
    while (parent != Node::Null() && here_ == parent->left) {
      here_ = parent;
      parent = here_->parent;
    }
    here_ = parent;
  }
  return *this;
}

template <RBTreeNodeType T, bool D>
typename RBTree<T, D>::ConstIterator::iterator_type
RBTree<T, D>::ConstIterator::operator--(int) {
  iterator_type temp = *this;
  this->operator--();
  return temp;
}

template <RBTreeNodeType T, bool D>
bool RBTree<T, D>::ConstIterator::operator==(const iterator_type& other) const {
  return here_ == other.here_;
}

template <RBTreeNodeType T>
using MultiSet = RBTree<T>;

template <RBTreeNodeType T>
using Set = RBTree<T, false>;

}  // namespace mirage::base

#endif  // MIRAGE_BASE_CONTAINER_SET
