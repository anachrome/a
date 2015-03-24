#include <iostream>
#include <string>

#include <vector>
#include <algorithm>
#include <cctype> // fuck this bullshit

class Trie {
private:
    struct Node {
        std::vector<Node> daughters;
        char val; // ???
        bool end = false; // ???

        void showall(std::string) const;
    };

    Node root;

public:
    void insert(std::string word);
    void showall(std::string word); // DEBUGING GUY
};

void Trie::insert(std::string word) {
    Node *node = &this->root; // this is a stupid fucking name
    Node *next = nullptr;

    for (char c : word) {
        next = nullptr;

        auto inext = std::lower_bound(
            node->daughters.begin(), node->daughters.end(),
            c, [](const Node &a, const char &b){ return a.val < b; });

        if (inext == node->daughters.cend() || inext->val != c) {
            next = &*node->daughters.emplace(inext);
            next->val = c;
        } else {
            next = &*inext;
        }

        node = next;
    }

    next->end = true;
}

void Trie::Node::showall(std::string word) const {
    for (const Node &n : daughters) {
        if (n.end) {
            //std::cout << word << n.val << std::endl;
        }

        std::string neword = word;
        neword.append(1, n.val);
        n.showall(neword);
    }
}

void Trie::showall(std::string word) {
    root.showall(word);
}

int main() {
    Trie dict;

    std::string word;
    while (std::getline(std::cin, word)) {
        if (std::any_of(word.cbegin(), word.cend(),
            [](const char &c) { return !std::isalpha(c); })) {
            continue;
        }

        dict.insert(word);
    }

    dict.showall("");

    return 0;
}
