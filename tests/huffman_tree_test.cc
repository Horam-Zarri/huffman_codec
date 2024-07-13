#include <gtest/gtest.h>
#include <map>
#include "huffman_tree.h"
// Demonstrate some basic assertions.
TEST(HuffmanTreeTest, AlphabetOnlyBasic) {
    std::map<char, uint64_t> mp;
    mp['a'] = 45;
    mp['b'] = 13;
    mp['c'] = 12;
    mp['d'] = 16;
    mp['e'] = 9;
    mp['f'] = 5;

    auto res = huffman_tree::huffman_table(std::move(mp));

    EXPECT_EQ(res['a'], "0");
    EXPECT_EQ(res['b'], "101");
    EXPECT_EQ(res['c'], "100");
    EXPECT_EQ(res['d'], "111");
    EXPECT_EQ(res['e'], "1101");
    EXPECT_EQ(res['f'], "1100");
}

TEST(HuffmanTreeTest, AlphabetOnlyAdvanced) {
    std::map<char, uint64_t> mp;

    constexpr int ALPHA_COUNT_EN = 26;
    int A = 'A';
    for (int i = 0; i < ALPHA_COUNT_EN; ++i) {
        mp[char(A + i)] = i + 1;
    }

    std::map<char, std::string> res = huffman_tree::huffman_table(std::move(mp));
    std::map<std::string, char> rev_res;
    for (const auto& [ch, repr]: res)
        rev_res[repr] = ch;

    auto miku = res['M'] + res['I'] + res['K'] + res['U'];

    std::string curr = "";
    std::string decoded = "";
    for (const char m : miku) {
        curr += m;
        if (rev_res[curr]) {
            decoded += rev_res[curr];
            curr = "";
        }
    }

    EXPECT_EQ(mp.size(), ALPHA_COUNT_EN);
    EXPECT_EQ(decoded, "MIKU");
}

TEST(HuffmanTreeTest, AlphaNumWS) {

    std::map<char, uint64_t> mp;

    mp['a'] = 6;
    mp[' '] = 5;
    mp['7'] = 7;
    mp['b'] = 2;
    mp['\n'] = 11;
    mp['9'] = 9;

    auto res = huffman_tree::huffman_table(std::move(mp));

    EXPECT_EQ(res['a'], "110");
    EXPECT_EQ(res['b'], "000");
    EXPECT_EQ(res['7'], "111");
    EXPECT_EQ(res['9'], "01");
    EXPECT_EQ(res[' '], "001");
    EXPECT_EQ(res['\n'], "10");
}