#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <map>
#include <algorithm>
#include <regex>
#include <locale>
#include <codecvt>

#include <getopt.h>

using namespace std;

typedef uint_fast64_t bitmask_t;
typedef vector<bitmask_t> word_t;
struct entry_t {
    string bytes;    // used to save on conversion time when (w)couting
    wstring letters; // the characters of the entry, in order
    word_t mask;     // the characters, represented as a vector of bitmasks
    bool same;       // true if this is an anagram of the previous entry
};
typedef list<entry_t>::const_iterator dict_iter_t;

// thanks stackoverflow
template <class Facet>
class deletable_facet : public Facet {
public:
    using Facet::Facet;
    ~deletable_facet() { }
};
typedef deletable_facet<codecvt_byname<wchar_t, char, mbstate_t>> locale_cvt;
wstring_convert<locale_cvt> converter(new locale_cvt(""));

// global options
namespace opt {
    enum mode_t { DROP, KEEP };
    string dictfilename = "/usr/share/dict/words";
    int min_letters = 0;
    int max_letters = 0;
    int min_words = 0;
    int max_words = 0;
    bool case_insensitive = false;
    bool show_words = false;
    wstring punctuation = L"";
    mode_t punctuation_mode = DROP;
    string word_separator = " ";
    string anagram_separator = "\n";
    string filter = "";
    mode_t filter_mode = KEEP;
}

bool count(map<wchar_t, bitmask_t> &letter_pos, wstring word, word_t &mask);
bool in(const word_t &needle, const word_t &haystack);
word_t remove(word_t needle, const word_t &haystack);
void print(const list<entry_t> &dict, vector<dict_iter_t> anagram);

// the sortedness of dict before calling this algorithm affects its speed
void anagram(const list<entry_t> &dict, const dict_iter_t &begin,
             word_t word, vector<dict_iter_t> &prefix) {
    if (!word.size() && prefix.size() >= opt::min_words)
        print(dict, prefix);

    if (opt::max_words && prefix.size() == opt::max_words)
        return;

    for (auto i = begin; i != dict.cend(); i++) {
        if (i->same)
            continue;
        if (in(i->mask, word)) {
            prefix.push_back(i);
            anagram(dict, i, remove(i->mask, word), prefix);
            prefix.pop_back();
        }
    }
}

void print(const list<entry_t> &dict, vector<dict_iter_t> anagram) {
    vector<pair<dict_iter_t, bool>> begin;
    for (auto i : anagram) {
        bool same = begin.size() && i->mask == begin.back().first->mask;
        begin.push_back({i, same});
    }

    while (true) {
        if (anagram.size()) {
            cout << (anagram[0]->bytes);
            for (int i = 1; i < anagram.size(); i++)
                cout << opt::word_separator << anagram[i]->bytes;
        }
        cout << opt::anagram_separator;

        int i
        for (i = anagram.size(); i > 0; i--)
            if (++anagram[i - 1] != dict.cend() && anagram[i - 1]->same)
                break;
        if (i - 1 < 0)
            return;
        for (; i < anagram.size(); i++)
            anagram[i] = begin[i].second ? anagram[i - 1] : begin[i].first;
    }
}

void anagram(wistream &dictfile, wstring want) {
    if (opt::case_insensitive)
        transform(want.begin(), want.end(), want.begin(), ::tolower);

    map<wchar_t, bitmask_t> letter_pos;

    bitmask_t letter_hash = 0x1;
    int alphabet = 0;
    for (wchar_t c : want) {
        if (opt::case_insensitive)
            c = tolower(c);
        if (letter_pos.find(c) != letter_pos.cend())
            continue;
        letter_pos[c] = letter_hash;
        letter_hash <<= 1;
        alphabet++;
    }

    // if this returns false, there is a bug in the program
    word_t cwant;
    count(letter_pos, want, cwant);

    list<entry_t> dict;
    regex re(opt::filter, regex_constants::extended);
    wstring wword;
    while (getline(dictfile, wword)) {
        string word = converter.to_bytes(wword);

        switch (opt::filter_mode) {
        case opt::DROP:
            if(regex_search(word, re))
                continue;
            break;
        case opt::KEEP:
            if(!regex_search(word, re))
                continue;
            break;
        }

        if (opt::case_insensitive)
            transform(wword.begin(), wword.end(), wword.begin(), ::tolower);

        switch (opt::punctuation_mode) {
        case (opt::KEEP):
            wword.erase(remove_if(wword.begin(), wword.end(),
                [](wchar_t c) {
                    //return find(punctuation.cbegin(), punctuation.cend(),
                    //    c) == punctuation.cend();
                    return opt::punctuation.find(c) == opt::punctuation.npos;
                }), wword.end());
            break;
        case (opt::DROP):
            wword.erase(remove_if(wword.begin(), wword.end(),
                [](wchar_t c) {
                    //return find(punctuation.cbegin(), punctuation.cend(),
                    //    c) != punctuation.cend();
                    return opt::punctuation.find(c) != opt::punctuation.npos;
                }), wword.end());
            break;
        }

        if (!wword.size())
            continue;

        if (opt::max_letters && wword.size() > opt::max_letters)
            continue;
        if (wword.size() < opt::min_letters)
            continue;

        word_t cword;
        if (!count(letter_pos, wword, cword))
            continue;
        if (!in(cword, cwant))
            continue;

        dict.push_back({ word, wword, cword, false });
    }

    dict.sort([](entry_t a, entry_t b) {
        if (a.letters.size() == b.letters.size())
            return a.mask < b.mask;
        else
            return a.letters.size() > b.letters.size();
    });

    for (auto i = dict.begin(); i != --dict.end();) {
        auto now = i->mask;
        auto next = (++i)->mask;
        if (now == next) {
            i->same = true;
        }
    }

    if (opt::show_words) {
        for (auto e : dict)
            cout << e.bytes << endl;
        return;
    }

    vector<dict_iter_t> prefix;
    anagram(dict, dict.cbegin(), cwant, prefix);
}

int main(int argc, char **argv) {
    // mutually exclusive options
    bool filter_drop = false, filter_keep = false;
    bool punctuation = false, alphabet = false;
    bool show_words = false, max_words = false, min_words = false;

    auto shorts = "d:l:L:w:W:isp:a:,:n:f:k:h";
    struct option longs[] = {
        /* name                 has_arg            flag     val */
        {  "dict",              required_argument, nullptr, 'd' },
        {  "min-letters",       required_argument, nullptr, 'l' },
        {  "max-letters",       required_argument, nullptr, 'L' },
        {  "max-words",         required_argument, nullptr, 'w' },
        {  "min-words",         required_argument, nullptr, 'W' },
        {  "ignore-case",       no_argument,       nullptr, 'i' },
        {  "show-words",        no_argument,       nullptr, 's' },
        {  "punctuation",       required_argument, nullptr, 'p' },
        {  "alphabet",          required_argument, nullptr, 'a' },
        {  "word-separator",    required_argument, nullptr, ',' },
        {  "anagram-separator", required_argument, nullptr, 'n' },
        {  "filter-drop",       required_argument, nullptr, 'f' },
        {  "filter-keep",       required_argument, nullptr, 'k' },
        {  "help",              no_argument,       nullptr, 'h' },
        { 0, 0, 0, 0 } /* end of list */
    };

    string prog = argv[0];
    string usage = "usage: " + string(argv[0]) + R"( [OPTION] PHRASE
find anagrams of PHRASE

  -d,  --dict=FILE              FILE contains a list of words which will be
                                  used to find anagrams of PHRASE;
                                  default is /usr/share/dict/words
  -l,  --min-letters=N          ignore words with less than N letters
  -L,  --max-letters=N          ignore words with more than N letters
  -w,  --max-words=N            do not generate anagrams with more than N words
  -W,  --min-words=N            do not generate anagrams with less than N words
  -f,  --filter-drop=REGEX      REGEX is an extended posix regex;
                                  words matching it are ignored
  -k,  --filter-keep=REGEX      REGEX is an extended posix regex;
                                  words not matching it are ignored
  -p,  --punctuation=LETTERS    LETTERS is a string representing a set of
                                  characters which are ignored when deciding
                                  whether two strings are anagrams
  -a,  --alphabet=LETTERS       LETTERS is a string representing a set of
                                  characters; any character not in LETTERS is
                                  ignored when deciding whether two strings are
                                  anagrams
  -i,  --ignore-case            ignore case when deciding whether two strings
                                  are anagrams
  -,,  --word-separator=STR     output STR between words in anagrams
  -n,  --anagram-separator=STR  output STR between anagrams
  -s,  --show-words             do not output anagrams; instead, show all of
                                  the words in the dictionary that can be made
                                  out of letters in PHRASE
  -h,  --help                   print this help message
)";

    int opt;
    while ((opt = getopt_long(argc, argv, shorts, longs, nullptr)) != -1) {
        if (opt == '?') {
            cerr << usage;
            return 1;
        } else {
            switch (opt) {
            case 'd':
                opt::dictfilename = optarg;
                break;
            case 'l':
                opt::min_letters = stoi(optarg);
                break;
            case 'L':
                opt::max_letters = stoi(optarg);
                break;
            case 'w':
                opt::max_words = stoi(optarg);
                max_words = true;
                break;
            case 'W':
                opt::min_words = stoi(optarg);
                min_words = true;
                break;
            case 'i':
                opt::case_insensitive = true;
                break;
            case 's':
                opt::show_words = true;
                show_words = true;
                break;
            case 'p':
                opt::punctuation = converter.from_bytes(optarg);
                punctuation = true;
                break;
            case 'a':
                opt::punctuation = converter.from_bytes(optarg);
                opt::punctuation_mode = opt::KEEP;
                alphabet = true;
                break;
            case ',':
                opt::word_separator = optarg;
                break;
            case 'n':
                opt::anagram_separator = optarg;
                break;
            case 'f':
                opt::filter = optarg;
                opt::filter_mode = opt::DROP;
                filter_drop = true;
                break;
            case 'k':
                opt::filter = optarg;
                filter_keep = true;
                break;
            case 'h':
                cerr << usage;
                return 0;
            }
        }
    }

    argc -= optind;
    argv += optind;

    if (punctuation && alphabet) {
        cerr << prog << ": --punctuation and --alphabet "
             << " are mutually exclusive options\n";
        return 1;
    } else if (filter_keep && filter_drop) {
        cerr << prog << ": --filter-drop and --filter-keep "
             << "are mutually exclusive\n";
        return 1;
    } else if (show_words && (min_words || max_words)) {
        cerr << prog << "warning: --min-words and --max-words "
             << "have no effect with --show-words\n";
    }

    wstring want;
    if (argc < 1) {
        cerr << prog << ": two few arguments: " << argc
             << " (exactly 1 is required)\n\n" << usage;
        return 1;
    } else if (argc > 1) {
        cerr << prog << ": two many arguments: " << argc
             << " (exactly 1 is required)\n\n" << usage;
        return 1;
    } else {
        want = converter.from_bytes(*argv);
    }

    wifstream dictfile(opt::dictfilename);
    dictfile.imbue(locale(""));
    if (!dictfile.good()) {
        cout << prog << ": unable to open file: " << opt::dictfilename << endl;
        return 1;
    }

    anagram(dictfile, want);

    return 0;
}

bool count(map<wchar_t, bitmask_t> &letter_pos, wstring word, word_t &mask) {
    for (wchar_t c : word) {
        if (letter_pos.find(c) == letter_pos.cend())
            return false;

        int index;
        for (index = mask.size(); index > 0; index--)
            if (mask[index - 1] & letter_pos[c])
                break;

        if (index == mask.size())
            mask.push_back(0x0);

        mask[index] |= letter_pos[c];
    }

    return true;
}

bool in(const word_t &needle, const word_t &haystack) {
    if (needle.size() > haystack.size())
        return false;

    for (int i = 0; i < needle.size(); i++)
        if ((needle[i] & haystack[i]) != needle[i])
            return false;

    return true;
}

// this is not guaranteed to do anything sane if needle isn't in haystack
word_t remove(word_t needle, const word_t &haystack) {
    word_t out = haystack;

    for (int i = 0; i < needle.size(); i++) {
        int end = out.size() - 1;

        while (needle[i] & ~0x0) {
            auto old_out = out[end];
            out[end] &= ~needle[i];
            needle[i] ^= (out[end] ^ old_out);

            if (!out[end])
                out.pop_back();

            end--;
        }
    }

    return out;
}
