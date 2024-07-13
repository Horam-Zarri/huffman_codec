//
// Created by horam on 7/10/2024.
//

#ifndef HUFFMANCODEC_HUFFMAN_CODEC_H
#define HUFFMANCODEC_HUFFMAN_CODEC_H

#include <iostream>
#include <functional>
#include <map>
#include <fstream>
#include <filesystem>
#include <cmath>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <ranges>

#include "huffman_tree.h"

class huffman_codec {
public:
    huffman_codec(): frequency_map{}, huffman_table{}, cond_var() {}

    void encode(const std::string_view input_file, const std::optional<std::string_view> output_file, const std::optional<std::string_view> table_file);
    void decode(const std::string_view input_file, const std::optional<std::string_view> output_file, const std::string_view table_file);

private:

    std::vector<char>::size_type BLOCK_SIZE = 0;
    enum class CodecType {Encoding, Decoding};

    void init_streams(const std::string_view& input_file, const std::string_view& output_file, const CodecType codec_type);

    void partition(const std::function<void(const std::vector<char>&&, std::mutex&, size_t)>& func, const CodecType codec_type);

    void write_huffman_encoded(const std::vector<char>&& data, std::mutex& mtx, size_t chunk_id);
    void write_huffman_decoded(const std::vector<char>&& data, std::mutex& mtx, size_t chunk_id);

    void fetch_char_freqs(const std::vector<char>&& data, std::mutex& mtx, [[maybe_unused]] size_t chunk_id);

    void read_huffman_table(std::ifstream& ifs);
    void write_huffman_table(std::ofstream& ofs);

    std::ifstream istrm;
    std::ofstream ostrm;
    std::map<char, uint64_t> frequency_map;
    std::map<char, std::string> huffman_table;

    std::map<std::string, char> rev_huffman_table;
    std::condition_variable cond_var;
    std::atomic_uint_fast32_t thread_chunk;
};


#endif //HUFFMANCODEC_HUFFMAN_CODEC_H
