/*
*    trie.h
*
*    (C) Denis Anikin 2020
*
*    Headers for trie
*
*/

#ifndef _trie_h_included_
#define _trie_h_included_

#include <iostream>
#include <string>
#include <vector>

struct trie_item
{
    char c_;
    unsigned int parent_;
    unsigned int next_;
    unsigned int child_;
};

// Make a flat trie out of words and returns handles to leafs
unsigned int make_trie(std::vector<std::string> &words,
                        std::vector<trie_item> &result,
                       std::vector<unsigned int> &leaf_refs);

// Restore a word by a handle to a trie leaf
std::string restore_word(const std::vector<trie_item> &flat_trie, unsigned int leaf_ref);



void print_flat_trie(const std::vector<trie_item> &flat_trie);

void print_leaf_refs(const std::vector<unsigned int> &leaf_refs);

void print_leaf_refs_with_words(const std::vector<trie_item> &flat_trie,
                                    const std::vector<unsigned int> &leaf_refs);

void print_mem_consumption(const std::vector<std::string> &words,
                           const std::vector<trie_item> &flat_trie);


unsigned int make_trie_impl(const std::vector<std::string> &words,
const std::string &prefix,
std::vector<trie_item> &result,
                            std::vector<unsigned int> &leaf_refs);

#endif

