#include "common.h"
void print_node(TSNode node, const std::string &source, int indent = 0) {
    // Print the current node's information.
    std::string prefix(indent, ' ');
    std::cout << prefix << "\\-- " << ts_node_type(node) << " (`";
    uint32_t start_byte = ts_node_start_byte(node);
    uint32_t end_byte = ts_node_end_byte(node);
    std::cout << source.substr(start_byte, end_byte - start_byte) << "`)" << std::endl;
    uint32_t child_count = ts_node_child_count(node);

    // Recursively print each child.
    for (uint32_t i = 0; i < child_count; i++) {
        TSNode child = ts_node_child(node, i);
        print_node(child, source, indent + 2);
    }

}