/*
*    trie.cpp
*
*    (C) Denis Anikin 2020
*
*    Impl for trie
*
*/

#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include "trie.h"

unsigned int make_trie_impl(const std::vector<std::string> &words,
                const std::string &prefix,
                std::vector<trie_item> &result,
                std::vector<unsigned int> &leaf_refs)
{
	/*
	*	TODO: improve!!!
	*	1.	We don't need next_, we just can put adjacent childs togather and store a marker of its count
	*		in the parent
	*	2.	We don't need parent_ for all childs except the first one
	*	3.	We can consider a child not just a char but a string as long as each char of the
	*		string has a single child
	*/

    std::string letters;
    size_t pref_size = prefix.size();
    
    /*bool b = false;
    if (prefix == " sahzavod 14")
    {
        b = true;
        std::cerr << "prefix='" << prefix << "'" << std::endl;
    }*/
    
    // Make the root record
    result.push_back({
        .c_ = (pref_size) ? prefix[pref_size-1] : (char)-1,
        .parent_ = (unsigned int)-1,
        .next_ = (unsigned int)-1,
        .child_ = (unsigned int)-1
    });
    unsigned int root_ref = result.size()-1;
    
    // Filter words by prefix and take letters right after the prefix
    auto start = std::lower_bound(words.begin(), words.end(), prefix);
    for (; start != words.end(); ++start)
    {
        auto &w = *start;
        //if (b)
          //  std::cerr << "word='" << w << "'" << std::endl;
        // Found a word started from the prefix
        if (w.size() >= pref_size && std::equal(prefix.begin(), prefix.end(), w.begin()))
        {
            if (w.size() == pref_size)
            {
                //std::cerr << "WORD MATCH PREF! '" << w << "'" << std::endl;
                leaf_refs.push_back(root_ref);
            }
            else
            {
                letters.push_back(w[pref_size]);
            }
        }
        else
            break;
    }
    
    // This is a leaf
    //if (letters.empty())
      //  leaf_refs.push_back(root_ref);
    
    //std::cerr << "letters.size()=" << letters.size() << std::endl;

    // Unique those letters
    std::sort(letters.begin(), letters.end());
    auto last = std::unique(letters.begin(), letters.end());
    
    // Make a trie for each letter
    unsigned int prev_ti_ref = (unsigned int)-1;
    for (auto l = letters.begin(); l != last; ++l)
    {
        unsigned cur_ref = result.size()-1;

        // Make a trie for everything beginning with prefix + this letter
        unsigned int child_ref = make_trie_impl(words, prefix + *l, result, leaf_refs);
        
        // Connect a child with the root
        result[child_ref].parent_ = root_ref;
        
        if (prev_ti_ref != (unsigned int)-1)
            // Link the prev and the current child
            result[prev_ti_ref].next_ = child_ref;
        else
            // Save the link to the first child at the root
            result[root_ref].child_ = child_ref;
        
        prev_ti_ref = child_ref;
    }
    
    // Return the ref to the root trie
    return root_ref;
}

unsigned int make_trie(std::vector<std::string> &words,
                        std::vector<trie_item> &result,
                        std::vector<unsigned int> &leaf_refs)
{
    std::sort(words.begin(), words.end());
    words.erase(std::unique(words.begin(), words.end()), words.end());
    return make_trie_impl(words, "", result, leaf_refs);
}

void print_flat_trie(const std::vector<trie_item> &flat_trie)
{
    int i = 0;
    for (auto &ti : flat_trie)
    {
        std::cerr << "[" << i++ << "], c=" << ti.c_
            << ", parent=" << ti.parent_
            << ", next=" << ti.next_
        << ", child=" << ti.child_ << std::endl;
    }
}

void print_leaf_refs(const std::vector<unsigned int> &leaf_refs)
{
    for (auto &ui : leaf_refs)
        std::cerr << "[" << ui << "]" << std::endl;
}

std::string restore_word(const std::vector<trie_item> &flat_trie, unsigned int leaf_ref)
{
    // Calc size
    std::string r;
    unsigned int t = leaf_ref;
    int n = 0;
    while (t != (unsigned int)-1)
    {
        t = flat_trie[t].parent_;
        ++n;
    }

    r.resize(n);
    
    // Going up again and fill "r"
    t = leaf_ref;
    while (t != (unsigned int)-1)
    {
        r[--n] = flat_trie[t].c_;
        t = flat_trie[t].parent_;
    }
    
    return r;
}

void print_leaf_refs_with_words(const std::vector<trie_item> &flat_trie,
                                    const std::vector<unsigned int> &leaf_refs)
{
//    for (auto &ui : leaf_refs)
  //      std::cerr << "[" << ui << "]" << ", " << restore_word(flat_trie, ui) << std::endl;
    std::cerr << "Total leafs: " << leaf_refs.size() << std::endl;
}

void print_mem_consumption(const std::vector<std::string> &words,
                            const std::vector<trie_item> &flat_trie)
{
    unsigned long long words_mem =  0;
    for (auto &i : words)
        words_mem += i.size();
    std::cerr << "words: " << words_mem << " bytes, flat_trie: " << flat_trie.size() * sizeof(trie_item)
    << " bytes" << std::endl;
    std::cerr << "Total words: " << words.size() << std::endl;
}
