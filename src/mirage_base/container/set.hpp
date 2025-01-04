#ifndef MIRAGE_BASE_CONTAINER_SET
#define MIRAGE_BASE_CONTAINER_SET

#include <concepts>

#include "mirage_base/container/array.hpp"
#include "mirage_base/util/aligned_memory.hpp"
#include "mirage_base/util/optional.hpp"

namespace mirage::base {

template <typename T>
concept RBTreeNodeType = std::move_constructible<T> && std::totally_ordered<T>;

template <RBTreeNodeType T, bool IS_DUPLICATE_ALLOWED>
class RBTree;

template <RBTreeNodeType T>
class RBTreeConstIterator;

template <RBTreeNodeType T>
struct RBTreeNode {
  enum Color { RED, BLACK };

  AlignedMemory<T> val;
  RBTreeNode* parent{Null()};
  RBTreeNode* left{Null()};
  RBTreeNode* right{Null()};
  Color color{RED};

  RBTreeNode(RBTreeNode&&) = delete;
  RBTreeNode(const RBTreeNode&) = delete;

  ~RBTreeNode() {
    // Only Null() will create uninitialized node, which is static
    if (this != Null()) [[likely]] {
      val.GetPtr()->~T();
    }
    parent = nullptr;
    left = nullptr;
    right = nullptr;
  }

  explicit RBTreeNode(T&& val) : val(std::move(val)) {}

 private:
  friend class RBTree<T, true>;
  friend class RBTree<T, false>;
  friend class RBTreeConstIterator<T>;

  static RBTreeNode* Null() {
    static RBTreeNode sentinel_null_node;
    return &sentinel_null_node;
  }

  RBTreeNode() : parent(this), left(this), right(this), color(BLACK) {}
};

template <RBTreeNodeType T>
class RBTreeConstIterator {
 public:
  using iterator_concept = std::bidirectional_iterator_tag;
  using iterator_category = std::bidirectional_iterator_tag;
  using iterator_type = RBTreeConstIterator;
  using difference_type = int64_t;
  using value_type = const T;
  using pointer = value_type*;
  using reference = value_type&;

  using Node = RBTreeNode<T>;

  RBTreeConstIterator() = default;
  ~RBTreeConstIterator() = default;

  RBTreeConstIterator(const RBTreeConstIterator& other) : here_(other.here_) {}

  explicit RBTreeConstIterator(Node& here) : here_(&here) {}

  iterator_type& operator=(const iterator_type& other) {
    if (this != &other) {
      here_ = other.here_;
    }
    return *this;
  }

  iterator_type& operator=(std::nullptr_t) {
    here_ = Node::Null();
    return *this;
  }

  reference operator*() const { return here_->val.GetConstRef(); }

  pointer operator->() const { return &here_->val.GetConstPtr(); }

  iterator_type& operator++() {
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

  iterator_type operator++(int) {
    iterator_type temp = *this;
    this->operator++();
    return temp;
  }

  iterator_type& operator--() {
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

  iterator_type operator--(int) {
    iterator_type temp = *this;
    this->operator--();
    return temp;
  }

  bool operator==(const iterator_type& other) const {
    return here_ == other.here_;
  }

 private:
  friend class RBTree<T, true>;
  friend class RBTree<T, false>;

  Node* here_{Node::Null()};
};

template <RBTreeNodeType T, bool IS_DUPLICATE_ALLOWED = true>
class RBTree {
 public:
  using Node = RBTreeNode<T>;
  using ConstIterator = RBTreeConstIterator<T>;

  using InsertResult =
      std::conditional_t<IS_DUPLICATE_ALLOWED, void, Optional<T>>;

  RBTree() = default;

  ~RBTree() { Clear(); }

  RBTree(std::initializer_list<T> list)
    requires std::copy_constructible<T>
  {
    for (const T& val : list) {
      Insert(T(val));
    }
  }

  InsertResult Insert(T&& val) {
    // Find insert place
    Node* parent = nullptr;
    Node* iter = root_;
    while (iter != Node::Null()) {
      parent = iter;
      if (val < iter->val.GetConstRef()) {
        iter = iter->left;
      } else if constexpr (IS_DUPLICATE_ALLOWED) {  // NOLINT: comptime branch
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

  Optional<T> Remove(const T& val) {
    // Find remove place
    Node* val_ptr = const_cast<Node*>(TryFind(val).here_);
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

  ConstIterator TryFind(const T& val) const {
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

  size_t Count(const T& val) const {
    if constexpr (IS_DUPLICATE_ALLOWED) {
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

  void Clear() {
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

  [[nodiscard]] bool IsEmpty() const { return size_ == 0; }

  [[nodiscard]] size_t GetSize() const { return size_; }

  ConstIterator begin() const {
    Node* iter = root_;
    while (iter->left != Node::Null()) {
      iter = iter->left;
    }
    return ConstIterator(*iter);
  }

  ConstIterator end() const { return ConstIterator(); }

 private:
  void RotateLeft(Node& node) {
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

  void RotateRight(Node& node) {
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

  constexpr InsertResult None() {
    if constexpr (IS_DUPLICATE_ALLOWED) {
      return;
    } else {
      return Optional<T>::None();
    }
  }

  Node* root_{Node::Null()};
  size_t size_{0};
};

template <RBTreeNodeType T>
using MultiSet = RBTree<T>;

template <RBTreeNodeType T>
using Set = RBTree<T, false>;

}  // namespace mirage::base

#endif  // MIRAGE_BASE_CONTAINER_SET
