#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

class Trie {
public:
    Trie();
    ~Trie();

    Trie(const Trie&) = delete;
    Trie& operator=(const Trie&) = delete;

    bool insert(const std::string& word, unsigned int startFreq =1);
    bool contains(const std::string& word) const;
    bool incrementFrequency(const std::string& word, unsigned int amount = 1);
    bool erase(const std::string& word);
    std::size_t size() const;
    bool empty() const;

    std::vector<std::string> complete(const std::string& prefix, std::size_t limit) const;
    std::vector<std::pair<char, unsigned long long>> predictNextLetters(const std::string& prefix, std::size_t limit) const;

private:
    static const int ALPHABET = 28;

    struct Node;

    Node*  root;
    std::size_t count;

    static int charIdx(char c);
    static char idxChar(int idx);
    static std::string norm(const std::string& text, bool allowTrailing = false);

    Node* findNode(const std::string& key) const;
    static bool hasChild(const Node* node);
    bool eraseRec(Node* node, const std::string& word, std::size_t depth);
    void destroy(Node* node);
    void gatherWords(const Node* node, std::string& cur, std::vector<std::pair<std::string, unsigned int>>& words) const;
    unsigned long long subtreeFreq(const Node* node) const;
};
