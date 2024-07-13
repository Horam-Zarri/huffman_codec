//
// Created by horam on 7/10/2024.
//

#ifndef HUFFMANCODEC_HUFFMAN_TREE_H
#define HUFFMANCODEC_HUFFMAN_TREE_H

#include <iostream>
#include <optional>
#include <cstdint>
#include <memory>
#include <queue>

template <typename T>
concept CharType =
std::same_as<std::remove_cv_t<T>, char> ||
std::same_as<std::remove_cv_t<T>, signed char> ||
std::same_as<std::remove_cv_t<T>, unsigned char> ||
std::same_as<std::remove_cv_t<T>, wchar_t> ||
std::same_as<std::remove_cv_t<T>, char8_t> ||
std::same_as<std::remove_cv_t<T>, char16_t> ||
std::same_as<std::remove_cv_t<T>, char32_t>;

class huffman_tree {
private:
    struct huffman_tree_node {
        huffman_tree_node(std::optional<char> ch, uint64_t freq) : ch{ch}, freq{freq}, right{nullptr}, left{nullptr} {}
        std::optional<char> ch;
        uint64_t freq;
        std::unique_ptr<huffman_tree_node> right;
        std::unique_ptr<huffman_tree_node> left;
    };

    template<template<typename, typename, typename...> class Map_Container, CharType K, std::integral V, typename... TArgs>
    static huffman_tree_node fetch_root(Map_Container<K, V, TArgs...>&& freq_map)
    {
        // Order nodes based on character frequencies
        struct custom_node_comparator {
            bool operator()(const huffman_tree_node &l, const huffman_tree_node &r) { return l.freq > r.freq; }
        };
        std::priority_queue<huffman_tree_node, std::vector<huffman_tree_node>, custom_node_comparator> q;

        for (const auto &[ch, fr]: freq_map)
            q.push(huffman_tree_node(ch, fr));

        const size_t len = q.size();
        for (size_t i = 0; i < len - 1; ++i) {
            huffman_tree_node x = std::move(const_cast<huffman_tree_node &>(q.top()));
            q.pop();
            huffman_tree_node y = std::move(const_cast<huffman_tree_node &>(q.top()));
            q.pop();

            q.push([&]() -> huffman_tree_node {
                huffman_tree_node new_node = huffman_tree_node(std::nullopt, x.freq + y.freq);
                new_node.left = std::make_unique<huffman_tree_node>(std::move(x));
                new_node.right = std::make_unique<huffman_tree_node>(std::move(y));
                return new_node;
            }());
        }

        huffman_tree_node root = std::move(const_cast<huffman_tree_node &>(q.top()));
        q.pop();

        return root;
    };

public:

    template<template<typename, typename, typename...> class Map_Container, CharType K, std::integral V, typename... TArgs, typename S = std::string>
    static Map_Container<K, S> huffman_table(Map_Container<K, V, TArgs...>&& freq_map)
    {
        Map_Container<K, S> huffman_table;
        std::unique_ptr<huffman_tree_node> root = std::make_unique<huffman_tree_node>(
                fetch_root(std::forward<Map_Container<K, V>>(freq_map)));

        //NOLINTBEGIN
        auto insert_node =
                [&huffman_table](auto&& insert_node, std::unique_ptr<huffman_tree_node> &node, S hm_code) -> void {
                    if (not node) {
                        return;
                    }
                    if (node->ch.has_value()) {
                        huffman_table.emplace(node->ch.value(), hm_code);
                    }
                    insert_node(insert_node, node->left, hm_code + "0");
                    insert_node(insert_node, node->right, hm_code + "1");
                };
        //NOLINTEND

        insert_node(insert_node, root, "");
        return huffman_table;
    }
};

#endif //HUFFMANCODEC_HUFFMAN_TREE_H
