#include "RedBlackTree.h"

#include <algorithm>
#include <limits>
#include <stdexcept>

struct RedBlackTree::Node {
    std::string word;
    unsigned int frequency;
    bool isRed;
    Node* left;
    Node* right;
    Node* parent;
};

RedBlackTree::RedBlackTree() {
    root_ = nullptr;
    size_ = 0;
}

RedBlackTree::~RedBlackTree() {
    Node* node = root_;
    while (node != nullptr) {
        if (node->left != nullptr) {
            node = node->left;
        } else if (node->right != nullptr) {
            node = node->right;
        } else {
            Node* parent = node->parent;
            if (parent == nullptr) {
                root_ = nullptr;
            } else if (parent->left == node) {
                parent->left = nullptr;
            } else {
                parent->right = nullptr;
            }
            delete node;
            node = parent;
        }
    }
}

std::string RedBlackTree::normalize(const std::string& text, bool allowTrailingSeparator) {
    if (text == "") {
        throw std::invalid_argument("Word must contain letters");
    }

    std::string normalized = text;
    bool previousWasSeparator = false;
    for (std::size_t index = 0; index < normalized.size(); ++index) {
        char& character = normalized[index];
        if (character >= 'A' && character <= 'Z') {
            character = static_cast<char>(character - 'A' + 'a');
        }
        if (character >= 'a' && character <= 'z') {
            previousWasSeparator = false;
        } else if (character == '-' || character == '\'') {
            if (index == 0 || previousWasSeparator) {
                throw std::invalid_argument("Apostrophes and hyphens must follow a letter");
            }
            previousWasSeparator = true;
        } else {
            throw std::invalid_argument("Word must contain only ASCII letters, apostrophes, and hyphens");
        }
    }
    if (previousWasSeparator && !allowTrailingSeparator) {
        throw std::invalid_argument("Word must end with a letter");
    }
    return normalized;
}

bool RedBlackTree::isRed(const Node* node) {
    if (node == nullptr) {
        return false;
    } else {
        return node->isRed;
    }
}

RedBlackTree::Node* RedBlackTree::find(const std::string& word) const {
    Node* node = root_;
    while (node != nullptr) {
        if (word == node->word) {
            return node;
        }
        if (word < node->word) {
            node = node->left;
        } else {
            node = node->right;
        }
    }
    return nullptr;
}

RedBlackTree::Node* RedBlackTree::lowerBound(const std::string& key) const {
    Node* node = root_;
    Node* result = nullptr;
    while (node != nullptr) {
        if (node->word >= key) {
            result = node;
            node = node->left;
        } else {
            node = node->right;
        }
    }
    return result;
}

RedBlackTree::Node* RedBlackTree::minimum(Node* node) const {
    while (node->left != nullptr) {
        node = node->left;
    }
    return node;
}

RedBlackTree::Node* RedBlackTree::successor(Node* node) const {
    if (node->right != nullptr) {
        return minimum(node->right);
    }
    Node* parent = node->parent;
    while (parent != nullptr && node == parent->right) {
        node = parent;
        parent = parent->parent;
    }
    return parent;
}

void RedBlackTree::rotateLeft(Node* node) {
    Node* right = node->right;
    node->right = right->left;
    if (right->left != nullptr) {
        right->left->parent = node;
    }
    right->parent = node->parent;
    if (node->parent == nullptr) {
        root_ = right;
    } else if (node == node->parent->left) {
        node->parent->left = right;
    } else {
        node->parent->right = right;
    }
    right->left = node;
    node->parent = right;
}

void RedBlackTree::rotateRight(Node* node) {
    Node* left = node->left;
    node->left = left->right;
    if (left->right != nullptr) {
        left->right->parent = node;
    }
    left->parent = node->parent;
    if (node->parent == nullptr) {
        root_ = left;
    } else if (node == node->parent->right) {
        node->parent->right = left;
    } else {
        node->parent->left = left;
    }
    left->right = node;
    node->parent = left;
}

void RedBlackTree::fixAfterInsert(Node* node) {
    while (isRed(node->parent)) {
        if (node->parent == node->parent->parent->left) {
            Node* uncle = node->parent->parent->right;
            if (isRed(uncle)) {
                node->parent->isRed = false;
                uncle->isRed = false;
                node->parent->parent->isRed = true;
                node = node->parent->parent;
            } else {
                if (node == node->parent->right) {
                    node = node->parent;
                    rotateLeft(node);
                }
                node->parent->isRed = false;
                node->parent->parent->isRed = true;
                rotateRight(node->parent->parent);
            }
        } else {
            Node* uncle = node->parent->parent->left;
            if (isRed(uncle)) {
                node->parent->isRed = false;
                uncle->isRed = false;
                node->parent->parent->isRed = true;
                node = node->parent->parent;
            } else {
                if (node == node->parent->left) {
                    node = node->parent;
                    rotateRight(node);
                }
                node->parent->isRed = false;
                node->parent->parent->isRed = true;
                rotateLeft(node->parent->parent);
            }
        }
    }
    root_->isRed = false;
}

bool RedBlackTree::insert(const std::string& word, unsigned int initialFrequency) {
    const std::string normalized = normalize(word);
    if (initialFrequency == 0) {
        throw std::invalid_argument("Initial frequency must be positive");
    }

    Node* parent = nullptr;
    Node* node = root_;
    while (node != nullptr) {
        parent = node;
        if (normalized == node->word) {
            return false;
        }
        node = normalized < node->word ? node->left : node->right;
    }

    Node* inserted = new Node{normalized, initialFrequency, true, nullptr, nullptr, parent};
    if (parent == nullptr) {
        root_ = inserted;
    } else if (normalized < parent->word) {
        parent->left = inserted;
    } else {
        parent->right = inserted;
    }
    ++size_;
    fixAfterInsert(inserted);
    return true;
}

bool RedBlackTree::incrementFrequency(const std::string& word, unsigned int amount) {
    const std::string normalized = normalize(word);
    if (amount == 0) {
        throw std::invalid_argument("Frequency increment must be positive");
    }
    Node* node = find(normalized);
    if (node == nullptr) {
        return false;
    }
    if (amount > std::numeric_limits<unsigned int>::max() - node->frequency) {
        throw std::overflow_error("Frequency would overflow");
    }
    node->frequency += amount;
    return true;
}

void RedBlackTree::transplant(Node* oldNode, Node* replacement) {
    if (oldNode->parent == nullptr) {
        root_ = replacement;
    } else if (oldNode == oldNode->parent->left) {
        oldNode->parent->left = replacement;
    } else {
        oldNode->parent->right = replacement;
    }
    if (replacement != nullptr) {
        replacement->parent = oldNode->parent;
    }
}

void RedBlackTree::fixAfterErase(Node* node, Node* parent, bool nodeIsLeft) {
    while (node != root_ && !isRed(node)) {
        if (nodeIsLeft) {
            Node* sibling = parent->right;
            if (isRed(sibling)) {
                sibling->isRed = false;
                parent->isRed = true;
                rotateLeft(parent);
                sibling = parent->right;
            }
            if (!isRed(sibling == nullptr ? nullptr : sibling->left) &&
                !isRed(sibling == nullptr ? nullptr : sibling->right)) {
                if (sibling != nullptr) {
                    sibling->isRed = true;
                }
                node = parent;
                parent = node->parent;
                nodeIsLeft = parent != nullptr && node == parent->left;
            } else {
                if (!isRed(sibling->right)) {
                    if (sibling->left != nullptr) {
                        sibling->left->isRed = false;
                    }
                    sibling->isRed = true;
                    rotateRight(sibling);
                    sibling = parent->right;
                }
                sibling->isRed = parent->isRed;
                parent->isRed = false;
                if (sibling->right != nullptr) {
                    sibling->right->isRed = false;
                }
                rotateLeft(parent);
                node = root_;
            }
        } else {
            Node* sibling = parent->left;
            if (isRed(sibling)) {
                sibling->isRed = false;
                parent->isRed = true;
                rotateRight(parent);
                sibling = parent->left;
            }
            if (!isRed(sibling == nullptr ? nullptr : sibling->right) &&
                !isRed(sibling == nullptr ? nullptr : sibling->left)) {
                if (sibling != nullptr) {
                    sibling->isRed = true;
                }
                node = parent;
                parent = node->parent;
                nodeIsLeft = parent != nullptr && node == parent->left;
            } else {
                if (!isRed(sibling->left)) {
                    if (sibling->right != nullptr) {
                        sibling->right->isRed = false;
                    }
                    sibling->isRed = true;
                    rotateLeft(sibling);
                    sibling = parent->left;
                }
                sibling->isRed = parent->isRed;
                parent->isRed = false;
                if (sibling->left != nullptr) {
                    sibling->left->isRed = false;
                }
                rotateRight(parent);
                node = root_;
            }
        }
    }
    if (node != nullptr) {
        node->isRed = false;
    }
}

bool RedBlackTree::erase(const std::string& word) {
    Node* target = find(normalize(word));
    if (target == nullptr) {
        return false;
    }

    Node* removed = target;
    bool removedWasRed = removed->isRed;
    Node* replacement = nullptr;
    Node* replacementParent = nullptr;
    bool replacementIsLeft = false;
    if (target->left == nullptr) {
        replacement = target->right;
        replacementParent = target->parent;
        replacementIsLeft = replacementParent != nullptr && target == replacementParent->left;
        transplant(target, replacement);
    } else if (target->right == nullptr) {
        replacement = target->left;
        replacementParent = target->parent;
        replacementIsLeft = replacementParent != nullptr && target == replacementParent->left;
        transplant(target, replacement);
    } else {
        removed = minimum(target->right);
        removedWasRed = removed->isRed;
        replacement = removed->right;
        if (removed->parent == target) {
            replacementParent = removed;
            replacementIsLeft = false;
        } else {
            replacementParent = removed->parent;
            replacementIsLeft = true;
            transplant(removed, replacement);
            removed->right = target->right;
            removed->right->parent = removed;
        }
        transplant(target, removed);
        removed->left = target->left;
        removed->left->parent = removed;
        removed->isRed = target->isRed;
    }

    delete target;
    --size_;
    if (!removedWasRed) {
        fixAfterErase(replacement, replacementParent, replacementIsLeft);
    }
    return true;
}

bool RedBlackTree::contains(const std::string& word) const {
    return find(normalize(word)) != nullptr;
}

std::vector<std::string> RedBlackTree::complete(const std::string& prefix, std::size_t limit) const {
    if (prefix.empty() || limit == 0) {
        return {};
    }
    const std::string normalized = normalize(prefix, true);
    std::vector<Node*> matches;
    for (Node* node = lowerBound(normalized); node != nullptr; node = successor(node)) {
        if (node->word.compare(0, normalized.size(), normalized) != 0) {
            break;
        }
        matches.push_back(node);
    }
    std::sort(matches.begin(), matches.end(), [](const Node* left, const Node* right) {
        if (left->frequency != right->frequency) {
            return left->frequency > right->frequency;
        }
        return left->word < right->word;
    });

    std::vector<std::string> results;
    results.reserve(std::min(limit, matches.size()));
    for (std::size_t index = 0; index < limit && index < matches.size(); ++index) {
        results.push_back(matches[index]->word);
    }
    return results;
}

std::size_t RedBlackTree::size() const {
    return size_;
}

bool RedBlackTree::empty() const {
    return size_ == 0;
}

