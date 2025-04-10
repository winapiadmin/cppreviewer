#include "patterns/constant_evaluator.h"  // Include our evaluator
extern std::vector<std::string> collect_return_values(TSNode node, const std::string& code, const std::string& function_name, bool in_function = false);
#include <fstream>
// Read file content into a string
std::string read_file(const std::string &path) {
    std::ifstream in(path);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}
int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: xrefparser <source.cpp>\n";
        return 1;
    }

    std::string source_code = read_file(argv[1]);

    // Initialize Tree-sitter C++ parser
    TSParser *parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_cpp());

    // Parse the source code into a syntax tree
    TSTree *tree = ts_parser_parse_string(
        parser,
        nullptr,
        source_code.c_str(),
        source_code.length()
    );

    TSNode root = ts_tree_root_node(tree);

    // Collect functions and cross-references
    std::vector<std::string> returns=collect_return_values(root, source_code, "main", false);
    for (const auto &return_value : returns) {
        std::cout << "Return value: " << return_value << "\n";
    }
    // Clean up
    ts_tree_delete(tree);
    ts_parser_delete(parser);
    return 0;
}
