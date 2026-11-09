#pragma once
// Glue between the HTTP layer and the (unmodified) backend Trie.
// No httplib dependency here, so it can be unit-tested on its own.

#include "RedBlackTree.h"
#include "Trie.h"

#include <fstream>
#include <limits>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

class AutocorrectService {
public:
    std::size_t loadWords(const std::string& path) {
        std::ifstream in(path);
        if (!in) throw std::runtime_error("cannot open word list: " + path);

        std::lock_guard<std::mutex> lock(mutex_);
        std::string line;
        std::size_t lineNumber = 0;
        while (std::getline(in, line)) {
            ++lineNumber;
            std::istringstream fields(line);
            std::string word;
            std::string frequencyText;
            std::string extra;
            if (!(fields >> word)) continue;
            if (!(fields >> frequencyText) || (fields >> extra)) {
                throw std::runtime_error("invalid word list entry on line " + std::to_string(lineNumber));
            }
            const unsigned int frequency = parseFrequency(frequencyText, lineNumber);
            try {
                const bool trieInserted = trie_.insert(word, frequency);
                const bool treeInserted = rbt_.insert(word, frequency);
                if (trieInserted != treeInserted) {
                    throw std::logic_error("data structures disagree on insertion");
                }
                if (!trieInserted) {
                    trie_.incrementFrequency(word, frequency);
                    rbt_.incrementFrequency(word, frequency);
                }
            } catch (const std::invalid_argument&) {
            }
        }
        if (in.bad()) throw std::runtime_error("could not read word list: " + path);
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
    static unsigned int parseFrequency(const std::string& text, std::size_t lineNumber) {
        unsigned int frequency = 0;
        const unsigned int maximum = std::numeric_limits<unsigned int>::max();
        for (char character : text) {
            if (character < '0' || character > '9') {
                throw std::runtime_error("invalid frequency on line " + std::to_string(lineNumber));
            }
            const unsigned int digit = static_cast<unsigned int>(character - '0');
            if (frequency > (maximum - digit) / 10) {
                throw std::overflow_error("frequency exceeds unsigned int on line " + std::to_string(lineNumber));
            }
            frequency = frequency * 10 + digit;
        }
        if (frequency == 0) {
            throw std::runtime_error("frequency must be positive on line " + std::to_string(lineNumber));
        }
        return frequency;
    }

    Trie trie_;
    RedBlackTree rbt_;
    std::mutex mutex_;  // Trie has no internal locking and httplib is multi-threaded
};
