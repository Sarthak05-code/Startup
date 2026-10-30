#include <iostream>
#include <memory>
#include <vector>


template <typename T> struct TreeNode {
  T data;
  std::vector<std::unique_ptr<TreeNode<T>>> children;

  TreeNode(T val) : data(val) {}

  // Add a child node
  TreeNode<T> *addChild(T val) {
    children.push_back(std::make_unique<TreeNode<T>>(val));
    return children.back().get();
  }
};

int main() {
  // Create root node with value 1
  TreeNode<int> root(1);

  // Add children to root
  TreeNode<int> *child1 = root.addChild(2);
  TreeNode<int> *child2 = root.addChild(3);

  // Add grandchild to child1
  child1->addChild(4);

  std::cout << "Root data: " << root.data << '\n';
  std::cout << "Child 1 data: " << root.children[0]->data << '\n';
  std::cout << "Grandchild data: " << root.children[0]->children[0]->data
            << '\n';

  return 0;
}
