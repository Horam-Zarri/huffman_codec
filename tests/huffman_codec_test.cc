#include "huffman_codec.h"
#include <gtest/gtest.h>
#include <source_location>
#include <utility>

class HuffmanCodecTest : public testing::Test {
protected:
    HuffmanCodecTest() = default;
    ~HuffmanCodecTest() override = default;

    void RunCodec(std::string file) {
        huffman_codec hmc;
        hmc.encode(file, std::nullopt, std::nullopt);
        file_no_ext = std::filesystem::path(file).replace_extension().string();
        hmc.decode(file_no_ext + "ENC.bin", file_no_ext + "Res.txt", file_no_ext + "Table.txt");
    }

    void TearDown() override {
        try {
            std::filesystem::remove(file_no_ext + "ENC.bin");
            std::filesystem::remove(file_no_ext + "Res.txt");
            std::filesystem::remove(file_no_ext + "Table.txt");

        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cout << "Could not delete temp test file: " << e.what() << std::endl;
        }
    }

    std::string file_no_ext;
};

static const std::string TEST_FILES_DIR = std::filesystem::path(std::source_location::current().file_name()).parent_path().string() + "/test_files";



template<typename InputIterator1, typename InputIterator2>
bool
range_equal(InputIterator1 first1, InputIterator1 last1,
            InputIterator2 first2, InputIterator2 last2)
{
    while(first1 != last1 && first2 != last2)
    {
        if(*first1 != *first2) return false;
        ++first1;
        ++first2;
    }
    return (first1 == last1) && (first2 == last2);
}

bool compare_files(const std::string& filename1, const std::string& filename2)
{
    std::ifstream file1(filename1);
    std::ifstream file2(filename2);

    std::istreambuf_iterator<char> begin1(file1);
    std::istreambuf_iterator<char> begin2(file2);

    std::istreambuf_iterator<char> end;

    bool eq = range_equal(begin1, end, begin2, end);

    file1.close();
    file2.close();

    return eq;
}


TEST_F(HuffmanCodecTest, Codec1M4C) {
    HuffmanCodecTest::RunCodec(TEST_FILES_DIR + "/1M4C.txt");
    EXPECT_TRUE(compare_files(TEST_FILES_DIR + "/1M4C.txt", TEST_FILES_DIR + "/1M4CRes.txt"));
}

TEST_F(HuffmanCodecTest, Codec100K16C) {
    HuffmanCodecTest::RunCodec(TEST_FILES_DIR + "/250K16C.txt");
    EXPECT_TRUE(compare_files(TEST_FILES_DIR + "/250K16C.txt", TEST_FILES_DIR + "/250K16CRes.txt"));
}

TEST_F(HuffmanCodecTest, Codec5M20C) {
    HuffmanCodecTest::RunCodec(TEST_FILES_DIR + "./5M30C.txt");
    EXPECT_TRUE(compare_files(TEST_FILES_DIR + "/5M30C.txt", TEST_FILES_DIR + "/5M30CRes.txt"));
}

TEST_F(HuffmanCodecTest, Codec1B5C) {
    try {
        HuffmanCodecTest::RunCodec(TEST_FILES_DIR + "./1B5C.txt");
        EXPECT_TRUE(compare_files(TEST_FILES_DIR + "/1B5C.txt", TEST_FILES_DIR + "/1B5CRes.txt"));
    } catch (...) {
        // Only assert test if the file exists (I ain't including a 1GB text file in GitHub bro)
    }
}

TEST_F(HuffmanCodecTest, CodecLibSource) {
    HuffmanCodecTest::RunCodec(TEST_FILES_DIR + "./LibSource.txt");
    EXPECT_TRUE(compare_files(TEST_FILES_DIR + "/LibSource.txt", TEST_FILES_DIR + "/LibSourceRes.txt"));
}

