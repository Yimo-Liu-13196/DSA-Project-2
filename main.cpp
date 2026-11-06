#include "RedBlackTree.h"

#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
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

int main() {
    try {
        testInsertionAndLookup();
        testCompletionAndFrequency();
        testPunctuation();
        testDeletion();
        std::cout << "All RedBlackTree checks passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "RedBlackTree check failed: " << error.what() << '\n';
        return 1;
    }
}
