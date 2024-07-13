#include <argumentum/argparse.h>
#include <chrono>
#include "huffman_codec.h"

class EncodeOptions : public argumentum::CommandOptions
{
public:
    std::string in_file;
    std::optional<std::string> out_file;
    std::optional<std::string> table_file;

    explicit EncodeOptions(std::string_view name) : CommandOptions(name) {}

    void execute(const argumentum::ParseResult& res) override
    {
        try {
            huffman_codec hmc;
            hmc.encode(in_file, out_file, table_file);
        }
        catch (const std::exception& e) {
            std::cout << "ENCODE FAILED: " << e.what() << std::endl
                << "Terminating..." << std::endl;
            std::exit(2);
        }

        std::cout << "Successfully encoded file!";
    }
protected:
    void add_parameters( argumentum::ParameterConfig& params) override
    {
        params.add_parameter(in_file, "INPUT_FILE").nargs(1).help("Input text file");
        params.add_parameter(out_file, "-o").maxargs(1).help("Output binary file");
        params.add_parameter(table_file, "-t").maxargs(1).help("Output table text file");
    }
};

class DecodeOptions: public argumentum::CommandOptions
{
public:
    std::string in_file;
    std::optional<std::string> out_file;
    std::string table_file;

    explicit DecodeOptions(std::string_view name) : CommandOptions(name) {}

    void execute(const argumentum::ParseResult& res) override
    {
        try {
            huffman_codec hmc;
            hmc.decode(in_file, out_file, table_file);
        }
        catch (const std::exception& e) {
            std::cout << "DECODE FAILED: " << e.what() << std::endl
                      << "Terminating..." << std::endl;
            std::exit(3);
        }

        std::cout << "Successfully decoded file!";
    }
protected:
    void add_parameters(argumentum::ParameterConfig& params) override
    {
        params.add_parameter(in_file, "INPUT_FILE").nargs(1).help("Input binary file");
        params.add_parameter(table_file, "TABLE_FILE").nargs(1).help("Input table text file");
        params.add_parameter(out_file, "-o").maxargs(1).help("Output text file");
    }
};


int init_cli( int argc, char** argv )
{
    auto parser = argumentum::argument_parser{};
    auto params = parser.params();
    parser.config().program( argv[0] ).description( "huffman_codec" );
    params.add_command<EncodeOptions>("encode").help("Encode a text file to binary");
    params.add_command<DecodeOptions>("decode").help("Decode a binary file to text");

    auto res = parser.parse_args( argc, argv, 1 );
    if ( !res )
        return 1;

    auto pcmd = res.commands.back();
    if ( !pcmd )
        return 1;

    auto start = std::chrono::high_resolution_clock::now();
    pcmd->execute( res );
    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>
        (stop - start);

    std::cout << std::endl << "Done after " << (long double)duration.count() / 1000000 << "s.";
    return 0;
}
