#pragma once
// Glue between the HTTP layer and the (unmodified) backend Trie.
// No httplib dependency here, so it can be unit-tested on its own.

#include "RedBlackTree.h"
#include "Trie.h"

#include <fstream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

class AutocorrectService {
public:
    // Seeds the trie from a word list (one word per line). Earlier lines are
    // treated as more common: frequency = (number of words) - line index.
    // Returns how many words were loaded.
    std::size_t loadWords(const std::string& path) {
        std::ifstream in(path);
        if (!in) throw std::runtime_error("cannot open word list: " + path);

        std::vector<std::string> words;
        for (std::string line; std::getline(in, line);) {
            while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) line.pop_back();
            if (!line.empty()) words.push_back(line);
        }

        std::lock_guard<std::mutex> lock(mutex_);
        for (std::size_t i = 0; i < words.size(); i++) {
            const unsigned int freq = static_cast<unsigned int>(words.size() - i);
            try {
                trie_.insert(words[i], freq);
                rbt_.insert(words[i], freq);  // same rules, so both accept/reject the same words
            } catch (const std::invalid_argument&) {
                // skip entries that are rejected (digits, spaces, ...)
            }
        }
        return trie_.size();
    }

    // "rbt" selects the RedBlackTree; anything else (including no value) selects the Trie.
    static bool isRbt(const std::string& engine) { return engine == "rbt"; }

    // complete(prefix, limit) on the chosen structure. A rejected prefix gives no suggestions.
    std::vector<std::string> suggest(const std::string& prefix, std::size_t limit,
                                     const std::string& engine = "trie") {
        std::lock_guard<std::mutex> lock(mutex_);
        try {
            return isRbt(engine) ? rbt_.complete(prefix, limit) : trie_.complete(prefix, limit);
        } catch (const std::invalid_argument&) {
            return {};
        }
    }

    // incrementFrequency(word) on BOTH structures so they always hold the same data and
    // switching between them never changes the ranking. False if unknown/invalid/overflowing.
    bool accept(const std::string& word) {
        std::lock_guard<std::mutex> lock(mutex_);
        try {
            const bool a = trie_.incrementFrequency(word);
            const bool b = rbt_.incrementFrequency(word);
            return a && b;
        } catch (const std::invalid_argument&) {
            return false;
        } catch (const std::overflow_error&) {
            return false;
        }
    }

    static std::string suggestionsJson(const std::vector<std::string>& words,
                                       const std::string& engine = "trie") {
        std::string out = std::string("{\"engine\":\"") + (isRbt(engine) ? "rbt" : "trie") +
                          "\",\"suggestions\":[";
        for (std::size_t i = 0; i < words.size(); i++) {
            if (i) out += ',';
            out += '"';
            for (char c : words[i]) {  // Trie words are [a-z'-] only, but escape anyway
                if (c == '"' || c == '\\') out += '\\';
                out += c;
            }
            out += '"';
        }
        return out + "]}";
    }

private:
    Trie trie_;
    RedBlackTree rbt_;
    std::mutex mutex_;  // Trie has no internal locking and httplib is multi-threaded
};
