#include "Trie.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

struct Trie::Node {
    Node* child[ALPHABET];
    bool isWord;
    unsigned int freq;

    Node() {
        for (int i = 0; i < ALPHABET; i++) {
            child[i] = nullptr;
        }
        isWord = false;
        freq = 0;
    }
};

Trie::Trie() {
    root= new Node();
    count=0;
}

Trie::~Trie() {
    destroy(root);
    root = nullptr;
}

int Trie::charIdx(char c){
    if (c >= 'a' && c <= 'z') {
        return c - 'a';
    }
    if (c == '\'') {
        return 26;
    }
    if (c == '-') {
        return 27;
    }
    return -1;
}

char Trie::idxChar(int idx) {
    if (idx >= 0 && idx < 26) {
        return (char)('a' + idx);
    }
    if (idx == 26) {
        return '\'';
    }
    if (idx == 27) {
        return '-';
    }
    return '\0';
}

std::string Trie::norm(const std::string& text, bool allowTrailing) {
    if (text.empty()) {
        throw std::invalid_argument("word must contain letters");
    }

    std::string res = "";
    bool lastSep = false;

    for (std::size_t i = 0; i < text.size(); i++) {
        char c = text[i];

        if (c>='A' && c <= 'Z') {
            c = c - 'A' + 'a';
        }

        if (c >= 'a' && c <= 'z') {
            lastSep = false;
        }
        else if (c == '\'' || c == '-') {
            if (i == 0 || lastSep) {
                throw std::invalid_argument("apostrophes and hyphens must follow a letter");
            }
            lastSep = true;
        }
        else {
            throw std::invalid_argument("word must contain only ascii letters, apostrophes, and hyphens");
        }

        res += c;
    }

    if (lastSep && !allowTrailing) {
        throw std::invalid_argument("word must end with a letter");
    }

    return res;
}

bool Trie::insert(const std::string& word, unsigned int startFreq) {
    std::string w = norm(word);
    if (startFreq == 0) {
        throw std::invalid_argument("initial frequency must be positive");
    }

    Node* cur = root;
    for (std::size_t i = 0; i < w.size(); i++) {
        int idx = charIdx(w[i]);
        if (cur->child[idx] == nullptr) {
            cur->child[idx] = new Node();
        }
        cur = cur->child[idx];
    }

    if (cur->isWord) {
        return false;
    }

    cur->isWord= true;
    cur->freq=startFreq;
    count++;
    return true;
}

Trie::Node* Trie::findNode(const std::string& key) const {
    Node* cur = root;
    for (std::size_t i = 0; i < key.size(); i++) {
        int idx = charIdx(key[i]);
        if (idx == -1 || cur->child[idx] == nullptr){
            return nullptr;
        }
        cur = cur->child[idx];
    }
    return cur;
}

bool Trie::contains(const std::string& word) const {
    Node* node = findNode(norm(word));
    return node != nullptr && node->isWord;
}

bool Trie::incrementFrequency(const std::string& word, unsigned int amount) {
    std::string w = norm(word);
    if (amount == 0) {
        throw std::invalid_argument("frequency increment must be positive");
    }

    Node* node = findNode(w);
    if (node == nullptr || !node->isWord) {
        return false;
    }

    if (amount > std::numeric_limits<unsigned int>::max() - node->freq){
        throw std::overflow_error("frequency would overflow");
    }

    node->freq += amount;
    return true;
}

bool Trie::hasChild(const Node* node) {
    for(int i = 0; i<ALPHABET; i++) {
        if (node->child[i] != nullptr) {
            return true;
        }
    }
    return false;
}

bool Trie::erase(const std::string& word){
    std::string w = norm(word);

    Node* node = findNode(w);
    if (node == nullptr || !node->isWord) {
        return false;
    }

    eraseRec(root, w, 0);
    count--;
    return true;
}

bool Trie::eraseRec(Node* node, const std::string& word, std::size_t depth) {
    if (depth == word.size()) {
        node->isWord = false;
        node->freq = 0;
        return !hasChild(node);
    }

    int idx = charIdx(word[depth]);
    Node* child = node->child[idx];


    bool removeChild = eraseRec(child, word, depth + 1);
    if (removeChild) {
        delete child;
        node->child[idx] = nullptr;
    }

    return !node->isWord&&!hasChild(node);
}

void Trie::destroy(Node* node){
    if(node == nullptr) {
        return;
    }
    for (int i = 0; i < ALPHABET; i++) {
        destroy(node->child[i]);
    }
    delete node;
}

std::size_t Trie::size() const {
    return count;
}

bool Trie::empty() const {
    return count == 0;
}

std::vector<std::string> Trie::complete(const std::string& prefix, std::size_t limit) const {
    std::vector<std::string> results;
    if (prefix.empty() || limit == 0) {
        return results;
    }

    std::string p = norm(prefix, true);
    Node* start = findNode(p);
    if (start == nullptr) {
        return results;
    }

    std::vector<std::pair<std::string, unsigned int>> words;
    std::string cur = p;
    gatherWords(start, cur, words);

    std::sort(words.begin(), words.end(), [](const std::pair<std::string, unsigned int>& a, const std::pair<std::string, unsigned int>& b) {
        if (a.second != b.second) {
            return a.second > b.second;
        }
        return a.first < b.first;
    });

    for (std::size_t i = 0; i < words.size() && i < limit; i++){
        results.push_back(words[i].first);
    }
    return results;
}

void Trie::gatherWords(const Node* node, std::string& cur, std::vector<std::pair<std::string, unsigned int>>& words) const {
    if (node->isWord) {
        words.push_back({cur, node->freq});
    }

    for (int i = 0; i < ALPHABET; i++) {
        if (node->child[i] != nullptr) {
            cur.push_back(idxChar(i));
            gatherWords(node->child[i], cur, words);
            cur.pop_back();
        }
    }
}

unsigned long long Trie::subtreeFreq(const Node* node) const {
    unsigned long long total = 0;
    if (node->isWord) {
        total += node->freq;
    }

    for (int i = 0; i < ALPHABET; i++) {
        if (node->child[i] != nullptr) {
            total += subtreeFreq(node->child[i]);
        }
    }
    return total;
}

std::vector<std::pair<char, unsigned long long>> Trie::predictNextLetters(const std::string& prefix, std::size_t limit) const {
    std::vector<std::pair<char, unsigned long long>> predicts;
    if (limit==0) {
        return predicts;
    }

    Node* start = root;
    if(!prefix.empty()) {
        start = findNode(norm(prefix, true));
    }
    if (start == nullptr) {
        return predicts;
    }

    for (int i = 0; i < ALPHABET; i++) {
        if (start->child[i] != nullptr) {
            unsigned long long score = subtreeFreq(start->child[i]);
            predicts.push_back({idxChar(i), score});
        }
    }

    std::sort(predicts.begin(), predicts.end(), [](const std::pair<char, unsigned long long>& a, const std::pair<char, unsigned long long>& b) {
        if (a.second != b.second) {
            return a.second > b.second;
        }
        return a.first < b.first;
    });

    if (predicts.size() > limit) {
        predicts.resize(limit);
    }
    return predicts;
}
