#include <tree_sitter/api.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <set>
#include <iomanip>
#include <algorithm>
extern "C" const TSLanguage *tree_sitter_cpp();

// Read file content into a string
std::string read_file(const std::string &path) {
    std::ifstream in(path);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}
std::string ts_node_string(TSNode node, const std::string& code){
    size_t start_byte = ts_node_start_byte(node);
    size_t end_byte = ts_node_end_byte(node);
    return code.substr(start_byte, end_byte - start_byte);
}

// Struct to hold collected function information
struct FunctionInfo {
    std::string name;
    std::string declaration;
};
void collect_functions(TSNode node, const std::string& code, std::vector<FunctionInfo>& functions) {
    std::string node_type = ts_node_type(node);

    // Removed special handling for template_declaration so that template functions get processed

    if (node_type == "function_definition" || node_type == "function_declaration") {
        // Try to find the function_declarator among the children
        TSNode declarator_node;
        uint32_t child_count = ts_node_child_count(node);
        for (uint32_t i = 0; i < child_count; ++i) {
            TSNode child = ts_node_child(node, i);
            if (std::string(ts_node_type(child)) == "function_declarator") {
                declarator_node = child;
                break;
            }
        }
        if (!ts_node_is_null(declarator_node)) {
            // Recursively search for the identifier node
            TSNode identifier_node = find_identifier(declarator_node);
            std::string function_name = ts_node_string(identifier_node, code);

            // Build declaration using start of first child and end of declarator_node
            TSNode first_child = ts_node_child(node, 0);
            size_t start_byte = ts_node_start_byte(first_child);
            size_t end_byte = ts_node_end_byte(declarator_node);
            std::string declaration_text = code.substr(start_byte, end_byte - start_byte) + ";";

            if (!function_name.empty()) {
                auto it = std::find_if(functions.begin(), functions.end(), [&function_name](const FunctionInfo& info) {
                    return info.name == function_name;
                });
                if (it == functions.end()) {
                    functions.push_back(FunctionInfo{function_name, declaration_text});
                }
            }
        }
    }

    // Recursively process all children
    uint32_t total_children = ts_node_child_count(node);
    for (uint32_t i = 0; i < total_children; ++i) {
        TSNode child = ts_node_child(node, i);
        collect_functions(child, code, functions);
    }
}


// Print the collected functions in a table-like format
void print_function_table(const std::vector<FunctionInfo>& functions) {
    std::cout << "-------------------------------------------------------------\n";
    std::cout << std::left << std::setw(30) << "Function Name" << std::setw(60) << "Declaration" << "\n";
    std::cout << "-------------------------------------------------------------\n";
    for (const auto& func : functions) {
        std::cout << std::left << std::setw(30) << func.name << std::setw(60) << func.declaration << "\n";
    }
    std::cout << "-------------------------------------------------------------\n";
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
    std::vector<FunctionInfo> functions;
    collect_functions(root, source_code, functions);

    // Print the function table
    print_function_table(functions);

    // Clean up
    ts_tree_delete(tree);
    ts_parser_delete(parser);

    return 0;
}
