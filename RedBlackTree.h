#pragma once

#include <cstddef>
#include <string>
#include <vector>

class RedBlackTree {
public:
    RedBlackTree();
    ~RedBlackTree();

    RedBlackTree(const RedBlackTree&) = delete;
    RedBlackTree& operator=(const RedBlackTree&) = delete;

    bool insert(const std::string& word, unsigned int initialFrequency = 1);
    bool incrementFrequency(const std::string& word, unsigned int amount = 1);
    bool erase(const std::string& word);
    bool contains(const std::string& word) const;
    std::vector<std::string> complete(const std::string& prefix, std::size_t limit) const;
    std::size_t size() const;
    bool empty() const;

private:
    struct Node;

    Node* root_;
    std::size_t size_;

    static std::string normalize(const std::string& text, bool allowTrailingSeparator = false);
    static bool isRed(const Node* node);
    Node* find(const std::string& word) const;
    Node* lowerBound(const std::string& key) const;
    Node* minimum(Node* node) const;
    Node* successor(Node* node) const;

    void rotateLeft(Node* node);
    void rotateRight(Node* node);
    void fixAfterInsert(Node* node);
    void transplant(Node* oldNode, Node* replacement);
    void fixAfterErase(Node* node, Node* parent, bool nodeIsLeft);
};
