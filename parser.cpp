#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <tree_sitter/api.h>

extern "C" TSLanguage *tree_sitter_cpp();

struct AnalysisResult {
    std::vector<std::string> warnings;
    void add_warning(const std::string &msg) {
        warnings.push_back(msg);
    }
};

std::string read_file(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Error: Cannot open file " + filename);
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::string buffer(size, '\0');
    if (!file.read(buffer.data(), size)) {
        throw std::runtime_error("Error: Cannot read file " + filename);
    }
    return buffer;
}

void analyze_code(const std::string &code, AnalysisResult &result) {
    TSParser *parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_cpp());
    TSTree *tree = ts_parser_parse_string(parser, nullptr, code.c_str(), code.size());
    TSNode root_node = ts_tree_root_node(tree);

    std::unordered_map<std::string, int> allocation_count;
    std::unordered_map<std::string, int> deallocation_count;
    std::unordered_map<std::string, std::string> allocated_pointers;
    std::unordered_set<std::string> freed_pointers;
    std::unordered_map<std::string, int> recursion_count;
    std::vector<TSNode> stack = {root_node};
    bool uses_dp_table = false;

    std::set<std::string> alloc_functions = {"malloc", "calloc", "realloc", "VirtualAlloc", "mmap", "new"};
    std::set<std::string> dealloc_functions = {"free", "VirtualFree", "munmap", "delete"};


    while (!stack.empty()) {
        TSNode node = stack.back();
        stack.pop_back();
        std::string type = ts_node_type(node);
        
        if (type == "init_declarator") {
            std::string var_name;
            for (uint32_t i = 0; i < ts_node_child_count(node); i++) {
                TSNode child = ts_node_child(node, i);
                std::string child_type = ts_node_type(child);
                if (child_type == "identifier") {
                    var_name = ts_node_string(child);
                } else if (child_type == "call_expression") {
                    TSNode func_node = ts_node_child(child, 0);
                    std::string func_name = ts_node_string(func_node);
                    if (alloc_functions.count(func_name)) {
                        allocation_count[var_name]++;
                        allocated_pointers[var_name] = func_name;
                    }
                } else if (child_type == "new_expression") {
                    allocation_count[var_name]++;
                    allocated_pointers[var_name] = "new";
                }
            }
        } else if (type == "delete_expression") {
            TSNode deleted_var_node;
            for (uint32_t i = 0; i < ts_node_child_count(node); ++i) {
                TSNode child = ts_node_child(node, i);
                std::string child_type = ts_node_type(child);
                if (child_type == "identifier" || child_type == "field_expression" || child_type == "subscript_expression") {
                    deleted_var_node = child;
                    break;
                }
            }
            if (!ts_node_is_null(deleted_var_node)) {
                std::string var_name = ts_node_string(deleted_var_node);
                deallocation_count[var_name]++;
                if (allocated_pointers.find(var_name) != allocated_pointers.end()) {
                    if (freed_pointers.find(var_name) != freed_pointers.end()) {
                        result.add_warning("Use-after-free detected: Attempt to delete already freed pointer " + var_name);
                    }
                    freed_pointers.insert(var_name);
                    allocated_pointers.erase(var_name);
                } else {
                    result.add_warning("Deleting a pointer that was never allocated: " + var_name);
                }
            }
        } else if (type == "call_expression") {
            TSNode func_node = ts_node_child(node, 0);
            std::string func_name = ts_node_string(func_node);
            // Handle deallocation functions like free()
            if (dealloc_functions.count(func_name)) {
                TSNode args_node;
                for (uint32_t i = 0; i < ts_node_child_count(node); i++) {
                    TSNode child = ts_node_child(node, i);
                    if (ts_node_type(child) == "argument_list") {
                        args_node = child;
                        break;
                    }
                }
                if (!ts_node_is_null(args_node)) {
                    TSNode first_arg = ts_node_child(args_node, 0);
                    if (!ts_node_is_null(first_arg)) {
                        std::string var_name = ts_node_string(first_arg);
                        deallocation_count[var_name]++;
                        if (allocated_pointers.find(var_name) != allocated_pointers.end()) {
                            if (freed_pointers.find(var_name) != freed_pointers.end()) {
                                result.add_warning("Use-after-free detected: " + var_name);
                            }
                            freed_pointers.insert(var_name);
                            allocated_pointers.erase(var_name);
                        } else {
                            result.add_warning("Deallocating unallocated pointer: " + var_name);
                        }
                    }
                }
            }
            // Removed greedy check for sort functions to focus on memory leaks.
            if (func_name.find("sort") != std::string::npos) {
                result.add_warning("Possible greedy approach detected. Consider DP if optimization is needed.");
            }
        } else if (type == "assignment_expression") {
            TSNode lhs = ts_node_child(node, 0);
            TSNode rhs = ts_node_child(node, 1);
            std::string rhs_type = ts_node_type(rhs);
            if (rhs_type == "call_expression") {
                TSNode func_node = ts_node_child(rhs, 0);
                std::string func_name = ts_node_string(func_node);
                if (alloc_functions.count(func_name)) {
                    if (ts_node_type(lhs) == "identifier") {
                        std::string var_name = ts_node_string(lhs);
                        allocation_count[var_name]++;
                        allocated_pointers[var_name] = func_name;
                    }
                }
            } else if (rhs_type == "new_expression") {
                if (ts_node_type(lhs) == "identifier") {
                    std::string var_name = ts_node_string(lhs);
        }
            }
        } else if (type == "function_declarator") {
            std::string func_name = ts_node_string(node);
            recursion_count[func_name]++;
        } else if (type == "subscript_expression") {
            uses_dp_table = true;
        }
        
        for (uint32_t i = 0; i < ts_node_child_count(node); i++) {
            stack.push_back(ts_node_child(node, i));
        }
    }
    
    for (const auto &[var_name, count] : allocation_count) {
        if (deallocation_count[var_name] < count) {
            result.add_warning("Potential memory leak: Variable `" + var_name + "` allocated " + std::to_string(count) + " times without corresponding deallocation.");
        }
    }
    
    for (const auto &[func_name, count] : recursion_count) {
        if (count > 1 && uses_dp_table) {
            result.add_warning("Dynamic Programming detected: Recursive function `" + func_name + "` combined with table usage.");
        }
    }
    
    ts_tree_delete(tree);
    ts_parser_delete(parser);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    try {
        std::string code = read_file(argv[1]);
        AnalysisResult result;
        analyze_code(code, result);

        for (const std::string &warning : result.warnings) {
            std::cout << "Warning: " << warning << "\n";
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}
