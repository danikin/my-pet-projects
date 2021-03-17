/*
*    trie.cpp
*
*    (C) Denis Anikin 2020
*
*    Impl for trie
*
*/

#include "trie.h"

int main()
{
    std::vector<trie_item> flat_trie;
    std::vector<unsigned int> leaf_refs;
    
    std::vector<std::string> words = {"aab", "abbc", "abz", "fau", "fauy", "y", "zzz"};
    
    make_trie(words, flat_trie, leaf_refs);

    std::cerr << "flat_trie" << std::endl;
    print_flat_trie(flat_trie);
    std::cerr << std::endl << "leaf_refs" << std::endl;
    print_leaf_refs_with_words(flat_trie, leaf_refs);
    
    print_mem_consumption(words, flat_trie);
    
    return 0;
}
