#include "common.h"
std::string ts_node_string(TSNode node, const std::string& code){
    size_t start_byte = ts_node_start_byte(node);
    size_t end_byte = ts_node_end_byte(node);
    return code.substr(start_byte, end_byte - start_byte);
}