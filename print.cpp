#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <tree_sitter/api.h>

extern "C" {
    TSLanguage *tree_sitter_cpp();
}

void print_node(TSNode node, const std::string &source, int indent = 0) {
    // Recursively print each child.
    uint32_t child_count = ts_node_child_count(node);
    for (uint32_t i = 0; i < child_count; i++) {
        TSNode child = ts_node_child(node, i);
        bool last = (i == child_count - 1);
        std::string prefix(indent, ' ');
        std::cout << prefix << (last ? "└── " : "├── ") << ts_node_type(child) << " (`";
        uint32_t start_byte = ts_node_start_byte(child);
        uint32_t end_byte = ts_node_end_byte(child);
        std::cout << source.substr(start_byte,end_byte-start_byte) << "`) "<< std::endl;
        print_node(child, source, indent + 4);
    }
    for (uint32_t i = 0; i < child_count; i++) {
        TSNode child = ts_node_child(node, i);
        print_node(child, source, indent + 2);
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    // Read file contents.
    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Error opening file: " << argv[1] << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    // Initialize parser with C++ language.
    TSParser *parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_cpp());

    // Parse the source code.
    TSTree *tree = ts_parser_parse_string(
        parser,
        nullptr,
        source.c_str(),
        source.size()
    );

    TSNode root_node = ts_tree_root_node(tree);

    // Print the syntax tree starting from the root.
    print_node(root_node, source);

    // Cleanup.
    ts_tree_delete(tree);
    ts_parser_delete(parser);

    return 0;
}