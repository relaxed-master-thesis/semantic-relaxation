#pragma once

#include <iostream>

// Enumeration for colors of nodes in Red-Black Tree

// Class template for Red-Black Tree
namespace bench {
template <typename T> class RedBlackTree {
  public:
	// Structure for a node in Red-Black Tree
	enum Color { RED, BLACK };
	struct Node {
		T data;
		Color color;
		Node *parent;
		Node *left;
		Node *right;

		// Constructor to initialize node with data and
		// color
		Node(T value)
			: data(value), color(RED), parent(nullptr), left(nullptr),
			  right(nullptr) {}
	};
	// Constructor: Initialize Red-Black Tree
	RedBlackTree() : root(nullptr) {}

	// Destructor: Delete Red-Black Tree
	~RedBlackTree() { deleteTree(root); }

	Node *getRoot() { return root; }

	// Public function: Insert a value into Red-Black Tree
	void insert(T key) {
		Node *node = new Node(key);
		Node *parent = nullptr;
		Node *current = root;
		while (current != nullptr) {
			parent = current;
			if (node->data < current->data)
				current = current->left;
			else
				current = current->right;
		}
		node->parent = parent;
		if (parent == nullptr)
			root = node;
		else if (node->data < parent->data)
			parent->left = node;
		else
			parent->right = node;
		fixInsert(node);
	}

	// Public function: Remove a value from Red-Black Tree
	void remove(T key) {
		Node *node = root;
		Node *z = nullptr;
		Node *x = nullptr;
		Node *y = nullptr;
		while (node != nullptr) {
			if (node->data == key) {
				z = node;
			}

			if (node->data <= key) {
				node = node->right;
			} else {
				node = node->left;
			}
		}

		if (z == nullptr) {
			std::cout << "Key not found in the tree" << std::endl;
			return;
		}

		y = z;
		Color yOriginalColor = y->color;
		if (z->left == nullptr) {
			x = z->right;
			transplant(root, z, z->right);
		} else if (z->right == nullptr) {
			x = z->left;
			transplant(root, z, z->left);
		} else {
			y = minValueNode(z->right);
			yOriginalColor = y->color;
			x = y->right;
			if (y->parent == z) {
				if (x != nullptr)
					x->parent = y;
			} else {
				transplant(root, y, y->right);
				y->right = z->right;
				y->right->parent = y;
			}
			transplant(root, z, y);
			y->left = z->left;
			y->left->parent = y;
			y->color = z->color;
		}
		delete z;
		if (yOriginalColor == BLACK) {
			fixDelete(x);
		}
	}

	// Public function: Print the Red-Black Tree
	void printTree() {
		if (root == nullptr)
			std::cout << "Tree is empty." << std::endl;
		else {
			std::cout << "Red-Black Tree:" << std::endl;
			printHelper(root, "", true);
		}
	}

  private:
	Node *root; // Root of the Red-Black Tree

	// Utility function: Left Rotation
	void rotateLeft(Node *&node) {
		Node *child = node->right;
		node->right = child->left;
		if (node->right != nullptr)
			node->right->parent = node;
		child->parent = node->parent;
		if (node->parent == nullptr)
			root = child;
		else if (node == node->parent->left)
			node->parent->left = child;
		else
			node->parent->right = child;
		child->left = node;
		node->parent = child;
	}

	// Utility function: Right Rotation
	void rotateRight(Node *&node) {
		Node *child = node->left;
		node->left = child->right;
		if (node->left != nullptr)
			node->left->parent = node;
		child->parent = node->parent;
		if (node->parent == nullptr)
			root = child;
		else if (node == node->parent->left)
			node->parent->left = child;
		else
			node->parent->right = child;
		child->right = node;
		node->parent = child;
	}

	// Utility function: Fixing Insertion Violation
	void fixInsert(Node *&node) {
		Node *parent = nullptr;
		Node *grandparent = nullptr;
		while (node != root && node->color == RED &&
			   node->parent->color == RED) {
			parent = node->parent;
			grandparent = parent->parent;
			if (parent == grandparent->left) {
				Node *uncle = grandparent->right;
				if (uncle != nullptr && uncle->color == RED) {
					grandparent->color = RED;
					parent->color = BLACK;
					uncle->color = BLACK;
					node = grandparent;
				} else {
					if (node == parent->right) {
						rotateLeft(parent);
						node = parent;
						parent = node->parent;
					}
					rotateRight(grandparent);
					swap(parent->color, grandparent->color);
					node = parent;
				}
			} else {
				Node *uncle = grandparent->left;
				if (uncle != nullptr && uncle->color == RED) {
					grandparent->color = RED;
					parent->color = BLACK;
					uncle->color = BLACK;
					node = grandparent;
				} else {
					if (node == parent->left) {
						rotateRight(parent);
						node = parent;
						parent = node->parent;
					}
					rotateLeft(grandparent);
					swap(parent->color, grandparent->color);
					node = parent;
				}
			}
		}
		root->color = BLACK;
	}

	// Utility function: Fixing Deletion Violation
	void fixDelete(Node *&node) {
		while (node != root && node->color == BLACK) {
			if (node == node->parent->left) {
				Node *sibling = node->parent->right;
				if (sibling->color == RED) {
					sibling->color = BLACK;
					node->parent->color = RED;
					rotateLeft(node->parent);
					sibling = node->parent->right;
				}
				if ((sibling->left == nullptr ||
					 sibling->left->color == BLACK) &&
					(sibling->right == nullptr ||
					 sibling->right->color == BLACK)) {
					sibling->color = RED;
					node = node->parent;
				} else {
					if (sibling->right == nullptr ||
						sibling->right->color == BLACK) {
						if (sibling->left != nullptr)
							sibling->left->color = BLACK;
						sibling->color = RED;
						rotateRight(sibling);
						sibling = node->parent->right;
					}
					sibling->color = node->parent->color;
					node->parent->color = BLACK;
					if (sibling->right != nullptr)
						sibling->right->color = BLACK;
					rotateLeft(node->parent);
					node = root;
				}
			} else {
				Node *sibling = node->parent->left;
				if (sibling->color == RED) {
					sibling->color = BLACK;
					node->parent->color = RED;
					rotateRight(node->parent);
					sibling = node->parent->left;
				}
				if ((sibling->left == nullptr ||
					 sibling->left->color == BLACK) &&
					(sibling->right == nullptr ||
					 sibling->right->color == BLACK)) {
					sibling->color = RED;
					node = node->parent;
				} else {
					if (sibling->left == nullptr ||
						sibling->left->color == BLACK) {
						if (sibling->right != nullptr)
							sibling->right->color = BLACK;
						sibling->color = RED;
						rotateLeft(sibling);
						sibling = node->parent->left;
					}
					sibling->color = node->parent->color;
					node->parent->color = BLACK;
					if (sibling->left != nullptr)
						sibling->left->color = BLACK;
					rotateRight(node->parent);
					node = root;
				}
			}
		}
		node->color = BLACK;
	}

	// Utility function: Find Node with Minimum Value
	Node *minValueNode(Node *&node) {
		Node *current = node;
		while (current->left != nullptr)
			current = current->left;
		return current;
	}

	// Utility function: Transplant nodes in Red-Black Tree
	void transplant(Node *&root, Node *&u, Node *&v) {
		if (u->parent == nullptr)
			root = v;
		else if (u == u->parent->left)
			u->parent->left = v;
		else
			u->parent->right = v;
		if (v != nullptr)
			v->parent = u->parent;
	}

	// Utility function: Helper to print Red-Black Tree
	void printHelper(Node *root, std::string indent, bool last) {
		if (root != nullptr) {
			std::cout << indent;
			if (last) {
				std::cout << "R----";
				indent += "   ";
			} else {
				std::cout << "L----";
				indent += "|  ";
			}
			std::string sColor = (root->color == RED) ? "RED" : "BLACK";
			std::cout << root->data << "(" << sColor << ")" << std::endl;
			printHelper(root->left, indent, false);
			printHelper(root->right, indent, true);
		}
	}

	// Utility function: Delete all nodes in the Red-Black
	// Tree
	void deleteTree(Node *node) {
		if (node != nullptr) {
			deleteTree(node->left);
			deleteTree(node->right);
			delete node;
		}
	}
};
} // namespace bench
