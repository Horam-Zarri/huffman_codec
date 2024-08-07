//
// Created by horam on 7/10/2024.
//

#include "huffman_codec.h"

void huffman_codec::encode(const std::string_view input_file,
                           const std::optional<std::string_view> output_file,
                           const std::optional<std::string_view> table_file)
                           {
    const std::string in_abs = std::filesystem::absolute(input_file).replace_extension().string();
    init_streams(input_file, output_file.value_or(in_abs + "ENC.bin"), CodecType::Encoding);

    // Bind function to "this" context
    std::function<void(const std::vector<char>&&, std::mutex&, size_t)> fp =
            std::bind(&huffman_codec::fetch_char_freqs, this,
                      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    partition(fp, CodecType::Encoding);

    huffman_table = huffman_tree::huffman_table(std::move(frequency_map));


    istrm.clear();
    istrm.seekg(0, std::ios::beg);

    fp = std::bind(&huffman_codec::write_huffman_encoded, this,
                   std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    partition(fp, CodecType::Encoding);

    // If table file output is provided make it owned else just append "Table.txt" to input file
    const auto t_file = table_file.transform([](auto tf) {return std::string(tf);})
            .value_or(in_abs + "Table.txt");

    std::ofstream table_strm(t_file);
    write_huffman_table(table_strm);
}


void huffman_codec::decode(const std::string_view input_file,
                           const std::optional<std::string_view> output_file,
                           const std::string_view table_file) {
    const std::string in_abs = std::filesystem::absolute(input_file).replace_extension().string();
    init_streams(input_file, output_file.value_or(in_abs + "DEC.txt"), CodecType::Decoding);

    if (!std::filesystem::exists(table_file)) {
        throw std::invalid_argument("Provided table_file_path path does not exist.");
    }

    std::string abs_file = std::filesystem::absolute(table_file).string();
    std::ifstream tstrm(abs_file);

    read_huffman_table(tstrm);

    for (const auto &e : huffman_table)
        rev_huffman_table[e.second] = e.first;

    // Bind function to "this" context
    const std::function<void(const std::vector<char>&&, std::mutex&, size_t)> fp =
            std::bind(&huffman_codec::write_huffman_decoded, this,
                      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    partition(fp, CodecType::Decoding);
}

void huffman_codec::init_streams(const std::string_view &input_file, const std::string_view &output_file,
                                 const huffman_codec::CodecType codec_type) {
    if (!std::filesystem::exists(input_file)) {
        throw std::invalid_argument("Provided input file path does not exist: " + std::string(input_file));
    }

    const auto in_ext = std::filesystem::path(input_file).extension();
    const auto out_ext = std::filesystem::path(output_file).extension();
    if (codec_type == CodecType::Encoding && (in_ext != ".txt" || out_ext != ".bin") ||
            codec_type == CodecType::Decoding && (in_ext != ".bin" || out_ext != ".txt"))
    {
        throw std::invalid_argument("File extensions invalid. Input/Output extension should be .txt/.bin for \n"
                                    "encode operations and .bin/.txt for decode operations, respectively.");
    }

    std::string abs_in_file = std::filesystem::absolute(input_file).string();
    std::string abs_out_file = std::filesystem::absolute(output_file).string();

    std::ifstream ifs(abs_in_file, codec_type == CodecType::Encoding ? std::ios::in : std::ios::binary);
    std::ofstream ofs(abs_out_file, codec_type == CodecType::Decoding ? std::ios::out : std::ios::binary);

    // Whatever that can happen lol...
    if (!ofs.is_open() || ofs.bad() || ofs.fail()) {
        throw std::invalid_argument("Cannot open output file to write.");
    }

    if (codec_type == CodecType::Encoding) {
        // Calculate chunk size for each thread
        ifs.seekg(0, std::ios::end);
        const auto ch_count = ifs.tellg();

        static const unsigned THREAD_COUNT = std::thread::hardware_concurrency();
        // Launch extra threads only when chunks are >256 bytes (to avoid
        // additional multithreading bookkeeping costs when files are small)
        BLOCK_SIZE = std::max((long double)256, std::floor((long double)ch_count / THREAD_COUNT));
        ifs.clear();
        ifs.seekg(0, std::ios::beg);
    }

    istrm = std::move(ifs);
    ostrm = std::move(ofs);
}

void huffman_codec::partition(const std::function<void(const std::vector<char> &&, std::mutex &, size_t)> &func,
                              const huffman_codec::CodecType codec_type) {

    std::mutex mtx;
    std::vector<std::thread> threads;
    size_t block_id = 0;
    while (!istrm.eof() && !istrm.fail())
    {
        std::vector<char> _buffer(BLOCK_SIZE);
        // chunk_id is read from file in decode or incremented using block_id in encode
        size_t chunk_id = 0;
        if (codec_type == CodecType::Decoding) {
            size_t byte_len = 0;
            constexpr size_t sz = sizeof(size_t);
            istrm.read(reinterpret_cast<char*>(&chunk_id), sz);
            istrm.read(reinterpret_cast<char*>(&byte_len), sz);

            if (byte_len == 0) break;
            // Chunk starts with an extra size_t for character length which is read at encode
            byte_len += sz;

            _buffer.resize(byte_len);
            istrm.read(_buffer.data(), byte_len);
            // some assert
        }
        if (codec_type == CodecType::Encoding) {
            istrm.read(_buffer.data(), BLOCK_SIZE);
            _buffer.resize(istrm.gcount());
            chunk_id = block_id;
        }

        // Extra check never hurts
        if (_buffer.empty()) break;

        ++block_id;

        // Need to reference wrap mutex, so it doesn't get moved.
        threads.push_back(std::thread(func, std::move(_buffer), std::ref(mtx), chunk_id));
    }

    for (auto&& t : threads)
        t.join();
}

void huffman_codec::write_huffman_encoded(const std::vector<char> &&data, std::mutex &mtx, size_t chunk_id) {
    size_t data_len = std::size(data);

    if (data_len == 0) {return;}
    std::vector<char> converted;

    uint8_t byte = 0;
    uint8_t offset = 0;

    // Bit manipulation bamboozle
    for (const char c : data) {
        const std::string hm_repr = huffman_table[c];
        const size_t repr_len = hm_repr.length();
        int ch_bin = std::stoi(hm_repr, 0, 2);

        for(int i = 0; i < repr_len; ++i) {
            byte <<= 1;
            byte |= (0b00000001 & (ch_bin >> (repr_len - i - 1)));
            if (++offset == 8) {
                converted.push_back((char)byte);
                byte = 0;
                offset = 0;
            }
        }
    }

    // If last byte was half way done shift to align
    if (offset > 0) {
        while (++offset <= 8) byte <<= 1;
        converted.push_back((char)byte);
    }

    size_t conv_len = converted.size();

    /*
     * Multithreading bookkeeping stuff to write:
     *
     * 1. Thread chunk's id to sync thread order in decode.
     * 2. Converted byte length (above) so partition function knows how long each chunk is to then read and
     *    pass to threads.
     * 3. Original character length of text file. An example is adequate to illustrate the necessity to have this information.
     *    Imagine a text file is only consisted of a's and b's, and we assign 0 to a and 1 to b. A chunk that
     *    is reading its last byte, suppose 10110000, cannot decide whether the last four 0's are to be translated
     *    as a's or they're just useless byte alignment (see above). The only way is to know how many characters were
     *    in the original chunk, so we can interpret the last byte (Again, this ambiguity only arises for last bytes).
     */

    std::unique_lock<std::mutex> lock(mtx);
    ostrm.write(reinterpret_cast<char*>(&chunk_id), sizeof(size_t));
    ostrm.write(reinterpret_cast<char*>(&conv_len), sizeof(size_t));
    ostrm.write(reinterpret_cast<char*>(&data_len), sizeof(size_t));
    ostrm.write(converted.data(), conv_len);
    lock.unlock();
}

void huffman_codec::fetch_char_freqs(const std::vector<char> &&data, std::mutex &mtx, size_t chunk_id) {
    std::unordered_map<char, uint64_t> freq_map_temp;
    for (auto c : data) {
        auto [mp_iterator, inserted] = freq_map_temp.try_emplace(c, 1);
        if (!inserted) {
            mp_iterator->second++;
        }
    }
    std::unique_lock<std::mutex> lock(mtx);
    for (const auto& [ch, fr] : freq_map_temp) {
        auto [mp_iterator, inserted] = frequency_map.try_emplace(ch, fr);
        if (!inserted) {
            mp_iterator->second += fr;
        }
    }
    lock.unlock();
}

void huffman_codec::write_huffman_decoded(const std::vector<char> &&data, std::mutex &mtx, size_t chunk_id) {

    // Retrieve character length
    uint64_t data_count = 0;
    for (size_t i = 0; i < 8; ++i)
        data_count |= static_cast<uint64_t>(static_cast<uint8_t>(data[i])) << (std::endian::native == std::endian::little ? 8 * i : i);

    std::vector<char> decrypted(data_count);

    size_t idx = 0, offset = 0;
    std::string curr;
    for (const char byte: data | std::views::drop(8)) {
        do {
            curr += (0b10000000 & (byte << offset)) ? "1" : "0";
            if (rev_huffman_table.contains(curr)) {
                decrypted[idx] = rev_huffman_table[curr];
                curr.clear();
                ++idx;
            }
        } while (++offset < 8 && idx < data_count);
        offset = 0;
    }

    std::unique_lock<std::mutex> lck(mtx);
    if (thread_chunk.load(std::memory_order_relaxed) != chunk_id)
        cond_var.wait(lck, [this, chunk_id]() {return thread_chunk.load(std::memory_order_acquire) == chunk_id;});
    ostrm.write(decrypted.data(), data_count);
    thread_chunk.store(chunk_id + 1, std::memory_order_release);
    lck.unlock();
    cond_var.notify_all();
}

void huffman_codec::read_huffman_table(std::ifstream &ifs) {
    std::string w, repr;

    while (ifs >> w >> repr) {
        char ch;

        if (w == "BR") ch = '\n';
        else if (w == "WS") ch = ' ';
        else ch = w.at(0);

        huffman_table.emplace(ch, repr);
    }
}

void huffman_codec::write_huffman_table(std::ofstream &ofs) {
    for (const auto& [ch, repr]: huffman_table) {
        std::string w;
        switch (ch) {
            case '\n':
                w = "BR";
                break;
            case ' ':
                w = "WS";
                break;
            default:
                w = ch;
        }
        ofs << w << ' ' << repr << std::endl;
    }
}
