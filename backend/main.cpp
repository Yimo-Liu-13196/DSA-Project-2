#include "RedBlackTree.h"
#include "Trie.h"

#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

void check(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void checkWords(const std::vector<std::string>& actual,
                const std::vector<std::string>& expected,
                const std::string& message) {
    check(actual == expected, message);
}

void testInsertionAndLookup() {
    RedBlackTree ascending;
    RedBlackTree descending;
    const std::vector<std::string> words{"ant", "bee", "cat", "dog", "eel"};

    check(ascending.empty(), "A new tree should be empty");
    check(ascending.size() == 0, "A new tree should have size zero");

    for (const std::string& word : words) {
        check(ascending.insert(word), "Ascending insertion failed");
    }
    for (std::size_t index = words.size(); index > 0; --index) {
        check(descending.insert(words[index - 1]), "Descending insertion failed");
    }

    for (const std::string& word : words) {
        check(ascending.contains(word), "Ascending lookup failed");
        check(descending.contains(word), "Descending lookup failed");
    }
    check(ascending.contains("BEE"), "Lookup should ignore letter case");
    check(!ascending.contains("fox"), "Missing word should not be found");
    check(!ascending.insert("ANT", 10), "Duplicate word should not be inserted");
    check(ascending.size() == words.size(), "Duplicate insertion changed the size");
    check(!ascending.empty(), "A populated tree should not be empty");
}

void testCompletionAndFrequency() {
    RedBlackTree tree;
    check(tree.insert("app"), "Could not insert app");
    check(tree.insert("apple", 3), "Could not insert apple");
    check(tree.insert("application"), "Could not insert application");
    check(tree.insert("apricot", 3), "Could not insert apricot");
    check(tree.insert("banana"), "Could not insert banana");
    check(tree.incrementFrequency("APPLICATION", 5), "Frequency update failed");
    check(!tree.incrementFrequency("missing"), "Missing word frequency was updated");

    checkWords(tree.complete("ap", 4),
               {"application", "apple", "apricot", "app"},
               "Completion ranking is incorrect");
    checkWords(tree.complete("AP", 2),
               {"application", "apple"},
               "Completion limit or case normalization is incorrect");
    checkWords(tree.complete("ban", 5), {"banana"}, "Single match is incorrect");
    checkWords(tree.complete("zzz", 5), {}, "Missing prefix should return no words");
    checkWords(tree.complete("", 5), {}, "Empty prefix should return no words");
    checkWords(tree.complete("ap", 0), {}, "Zero limit should return no words");

    bool rejected = false;
    try {
        tree.insert("zero", 0);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    check(rejected, "Zero initial frequency should be rejected");

    rejected = false;
    try {
        tree.incrementFrequency("app", 0);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    check(rejected, "Zero frequency increment should be rejected");

    RedBlackTree overflowTree;
    check(overflowTree.insert("maximum", std::numeric_limits<unsigned int>::max()),
          "Could not insert maximum frequency");
    bool overflowed = false;
    try {
        overflowTree.incrementFrequency("maximum");
    } catch (const std::overflow_error&) {
        overflowed = true;
    }
    check(overflowed, "Frequency overflow should be rejected");
}

void testPunctuation() {
    RedBlackTree tree;
    check(tree.insert("Don't", 4), "Could not insert apostrophe word");
    check(tree.insert("don-t", 2), "Could not insert hyphenated word");
    check(tree.insert("mother-in-law"), "Could not insert multi-hyphen word");
    check(tree.insert("rock'n'roll"), "Could not insert multi-apostrophe word");
    check(tree.contains("DON'T"), "Apostrophe lookup failed");
    checkWords(tree.complete("don'", 5), {"don't"}, "Apostrophe prefix failed");
    checkWords(tree.complete("don-", 5), {"don-t"}, "Hyphen prefix failed");
    checkWords(tree.complete("mother-", 5),
               {"mother-in-law"},
               "Trailing hyphen prefix failed");
    check(tree.incrementFrequency("DON-T", 3), "Hyphenated frequency update failed");
    checkWords(tree.complete("don", 2), {"don-t", "don't"},
               "Punctuation ranking failed");
    check(tree.erase("MOTHER-IN-LAW"), "Hyphenated deletion failed");
    check(!tree.contains("mother-in-law"), "Deleted hyphenated word was found");

    const std::vector<std::string> invalidWords{
        "-word", "'word", "word-", "word'", "a--b", "a''b", "a b"
    };
    for (const std::string& word : invalidWords) {
        bool rejected = false;
        try {
            tree.insert(word);
        } catch (const std::invalid_argument&) {
            rejected = true;
        }
        check(rejected, "Invalid word was accepted: " + word);
    }
}

void testDeletion() {
    RedBlackTree leafTree;
    check(leafTree.insert("b"), "Leaf test insertion failed");
    check(leafTree.insert("a"), "Leaf test insertion failed");
    check(leafTree.insert("c"), "Leaf test insertion failed");
    check(leafTree.erase("a"), "Leaf deletion failed");
    check(!leafTree.contains("a"), "Deleted leaf was found");
    check(!leafTree.erase("a"), "Deleting a missing word should return false");
    check(leafTree.size() == 2, "Leaf deletion size is incorrect");

    RedBlackTree oneChildTree;
    for (const std::string& word : std::vector<std::string>{"b", "a", "c", "d"}) {
        check(oneChildTree.insert(word), "One-child test insertion failed");
    }
    check(oneChildTree.erase("c"), "One-child deletion failed");
    check(oneChildTree.contains("d"), "Child was lost during deletion");
    check(oneChildTree.size() == 3, "One-child deletion size is incorrect");

    RedBlackTree twoChildTree;
    for (const std::string& word :
         std::vector<std::string>{"d", "b", "f", "a", "c", "e", "g"}) {
        check(twoChildTree.insert(word), "Two-child test insertion failed");
    }
    check(twoChildTree.erase("b"), "Two-child deletion failed");
    check(!twoChildTree.contains("b"), "Deleted two-child node was found");
    for (const std::string& word :
         std::vector<std::string>{"a", "c", "d", "e", "f", "g"}) {
        check(twoChildTree.contains(word), "A word was lost during two-child deletion");
    }

    RedBlackTree rootTree;
    check(rootTree.insert("root"), "Root test insertion failed");
    check(rootTree.erase("root"), "Root deletion failed");
    check(rootTree.empty(), "Tree should be empty after root deletion");
}

void checkPredictions(const std::vector<std::pair<char, unsigned long long>>& actual,
                      const std::vector<std::pair<char, unsigned long long>>& expected,
                      const std::string& message) {
    check(actual == expected, message);
}

void testTrieInsertionAndLookup() {
    Trie ascending;
    Trie descending;
    const std::vector<std::string> words{"ant", "bee", "cat", "dog", "eel"};

    check(ascending.empty(), "A new trie should be empty");
    check(ascending.size() == 0, "A new trie should have size zero");

    for (const std::string& word : words) {
        check(ascending.insert(word), "Trie ascending insertion failed");
    }
    for (std::size_t index = words.size(); index > 0; --index) {
        check(descending.insert(words[index - 1]), "Trie descending insertion failed");
    }

    for (const std::string& word : words) {
        check(ascending.contains(word), "Trie ascending lookup failed");
        check(descending.contains(word), "Trie descending lookup failed");
    }
    check(ascending.contains("BEE"), "Trie lookup should ignore letter case");
    check(!ascending.contains("fox"), "Missing word should not be found in trie");
    check(!ascending.contains("an"), "A prefix that was never inserted should not be found");
    check(!ascending.insert("ANT", 10), "Duplicate word should not be inserted into trie");
    check(ascending.size() == words.size(), "Duplicate insertion changed the trie size");
    check(!ascending.empty(), "A populated trie should not be empty");
}

void testTrieCompletionAndFrequency() {
    Trie trie;
    check(trie.insert("app"), "Could not insert app into trie");
    check(trie.insert("apple", 3), "Could not insert apple into trie");
    check(trie.insert("application"), "Could not insert application into trie");
    check(trie.insert("apricot", 3), "Could not insert apricot into trie");
    check(trie.insert("banana"), "Could not insert banana into trie");
    check(trie.incrementFrequency("APPLICATION", 5), "Trie frequency update failed");
    check(!trie.incrementFrequency("missing"), "Missing word frequency was updated in trie");
    check(!trie.incrementFrequency("appl"), "Prefix-only frequency was updated in trie");

    checkWords(trie.complete("ap", 4),
               {"application", "apple", "apricot", "app"},
               "Trie completion ranking is incorrect");
    checkWords(trie.complete("AP", 2),
               {"application", "apple"},
               "Trie completion limit or case normalization is incorrect");
    checkWords(trie.complete("ban", 5), {"banana"}, "Trie single match is incorrect");
    checkWords(trie.complete("zzz", 5), {}, "Missing prefix should return no words from trie");
    checkWords(trie.complete("", 5), {}, "Empty prefix should return no words from trie");
    checkWords(trie.complete("ap", 0), {}, "Zero limit should return no words from trie");

    bool rejected = false;
    try {
        trie.insert("zero", 0);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    check(rejected, "Zero initial frequency should be rejected by trie");

    rejected = false;
    try {
        trie.incrementFrequency("app", 0);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    check(rejected, "Zero frequency increment should be rejected by trie");

    Trie overflowTrie;
    check(overflowTrie.insert("maximum", std::numeric_limits<unsigned int>::max()),
          "Could not insert maximum frequency into trie");
    bool overflowed = false;
    try {
        overflowTrie.incrementFrequency("maximum");
    } catch (const std::overflow_error&) {
        overflowed = true;
    }
    check(overflowed, "Trie frequency overflow should be rejected");
}

void testTriePunctuation() {
    Trie trie;
    check(trie.insert("Don't", 4), "Could not insert apostrophe word into trie");
    check(trie.insert("don-t", 2), "Could not insert hyphenated word into trie");
    check(trie.insert("mother-in-law"), "Could not insert multi-hyphen word into trie");
    check(trie.insert("rock'n'roll"), "Could not insert multi-apostrophe word into trie");
    check(trie.contains("DON'T"), "Trie apostrophe lookup failed");
    checkWords(trie.complete("don'", 5), {"don't"}, "Trie apostrophe prefix failed");
    checkWords(trie.complete("don-", 5), {"don-t"}, "Trie hyphen prefix failed");
    checkWords(trie.complete("mother-", 5),
               {"mother-in-law"},
               "Trie trailing hyphen prefix failed");
    check(trie.incrementFrequency("DON-T", 3), "Trie hyphenated frequency update failed");
    checkWords(trie.complete("don", 2), {"don-t", "don't"},
               "Trie punctuation ranking failed");
    check(trie.erase("MOTHER-IN-LAW"), "Trie hyphenated deletion failed");
    check(!trie.contains("mother-in-law"), "Deleted hyphenated word was found in trie");

    const std::vector<std::string> invalidWords{
        "-word", "'word", "word-", "word'", "a--b", "a''b", "a b"
    };
    for (const std::string& word : invalidWords) {
        bool rejected = false;
        try {
            trie.insert(word);
        } catch (const std::invalid_argument&) {
            rejected = true;
        }
        check(rejected, "Invalid word was accepted by trie: " + word);
    }
}

void testTrieDeletion() {
    Trie prefixTrie;
    for (const std::string& word : std::vector<std::string>{"car", "cart", "care"}) {
        check(prefixTrie.insert(word), "Trie prefix test insertion failed");
    }
    check(prefixTrie.erase("car"), "Trie prefix-word deletion failed");
    check(!prefixTrie.contains("car"), "Deleted prefix word was found in trie");
    check(prefixTrie.contains("cart"), "Longer word was lost after prefix deletion");
    check(prefixTrie.contains("care"), "Longer word was lost after prefix deletion");
    check(prefixTrie.size() == 2, "Trie prefix deletion size is incorrect");

    Trie longerTrie;
    check(longerTrie.insert("car"), "Trie longer-word test insertion failed");
    check(longerTrie.insert("cart"), "Trie longer-word test insertion failed");
    check(longerTrie.erase("cart"), "Trie longer-word deletion failed");
    check(!longerTrie.contains("cart"), "Deleted longer word was found in trie");
    check(longerTrie.contains("car"), "Shorter word was lost after longer-word deletion");
    checkWords(longerTrie.complete("car", 5), {"car"}, "Trie branch was not cleaned up after deletion");

    check(!longerTrie.erase("ca"), "Erasing a prefix that is not a word should return false");
    check(!longerTrie.erase("dog"), "Erasing a missing word should return false");
    check(longerTrie.size() == 1, "Failed deletions changed the trie size");

    Trie emptyTrie;
    check(emptyTrie.insert("solo"), "Trie empty test insertion failed");
    check(emptyTrie.erase("solo"), "Trie single-word deletion failed");
    check(emptyTrie.empty(), "Trie should be empty after deleting its only word");
    check(!emptyTrie.erase("solo"), "Deleting an already deleted word should return false");
    check(emptyTrie.insert("solo"), "Trie should accept a word again after deletion");
    check(emptyTrie.contains("solo"), "Reinserted word was not found in trie");
}

void testTriePrediction() {
    Trie trie;
    check(trie.insert("cat", 5), "Could not insert cat into trie");
    check(trie.insert("car", 3), "Could not insert car into trie");
    check(trie.insert("cow", 2), "Could not insert cow into trie");
    check(trie.insert("dog"), "Could not insert dog into trie");

    checkPredictions(trie.predictNextLetters("c", 5),
                     {{'a', 8}, {'o', 2}},
                     "Next-letter scores or ranking are incorrect");
    checkPredictions(trie.predictNextLetters("", 5),
                     {{'c', 10}, {'d', 1}},
                     "Empty prefix should predict from the root");
    checkPredictions(trie.predictNextLetters("CA", 1),
                     {{'t', 5}},
                     "Prediction limit or case normalization is incorrect");
    checkPredictions(trie.predictNextLetters("cat", 5), {},
                     "A word with no longer words should predict nothing");
    checkPredictions(trie.predictNextLetters("x", 5), {},
                     "Missing prefix should predict nothing");
    checkPredictions(trie.predictNextLetters("c", 0), {},
                     "Zero limit should predict nothing");

    Trie tieTrie;
    check(tieTrie.insert("ab", 2), "Could not insert ab into trie");
    check(tieTrie.insert("ac", 2), "Could not insert ac into trie");
    checkPredictions(tieTrie.predictNextLetters("a", 5),
                     {{'b', 2}, {'c', 2}},
                     "Tied predictions should be in alphabetical order");

    Trie punctuationTrie;
    check(punctuationTrie.insert("don't", 4), "Could not insert don't into trie");
    check(punctuationTrie.insert("done"), "Could not insert done into trie");
    checkPredictions(punctuationTrie.predictNextLetters("don", 5),
                     {{'\'', 4}, {'e', 1}},
                     "Apostrophe should be predicted as a next letter");
}

int main() {
    try {
        testInsertionAndLookup();
        testCompletionAndFrequency();
        testPunctuation();
        testDeletion();
        std::cout << "All RedBlackTree checks passed.\n";
    } catch (const std::exception& error) {
        std::cerr << "RedBlackTree check failed: " << error.what() << '\n';
        return 1;
    }

    try {
        testTrieInsertionAndLookup();
        testTrieCompletionAndFrequency();
        testTriePunctuation();
        testTrieDeletion();
        testTriePrediction();
        std::cout << "All Trie checks passed.\n";
    } catch (const std::exception& error) {
        std::cerr << "Trie check failed: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
