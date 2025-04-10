#include "common.h"
#include "constant_evaluator.h"  // Include our evaluator

// Return value collector
// This function will collect the possible return values for a specific function
std::vector<std::string> collect_return_values(TSNode node, const std::string& code, const std::string& function_name, bool in_function = false) {
    // If we see a return_statement in a function, extract its value.
    if (!strcmp(ts_node_type(node), "return_statement") && in_function) {
        TSNode return_value = ts_node_child(node, 1);
        std::string return_value_text = ts_node_string(return_value, code);
        return std::vector<std::string>{return_value_text};
    }
    
    // Handle if-statements by evaluating their condition.
    if (!strcmp(ts_node_type(node), "if_statement")) {
        // Adjust these child indices according to the actual Tree-sitter C++ grammar.
        // For this example, assume:
        // child 0: "if" keyword
        // child 1: condition
        // child 2: then branch
        // child 3: optional "else" keyword
        // child 4: optional else branch
        TSNode condition = ts_node_child(node, 1);
        TSNode if_body   = ts_node_child(node, 2);
        
        // Create a simple variables map. Extend this if you need to evaluate identifiers.
        std::unordered_map<std::string, int> variables;
        if (is_constant_true(condition, code, variables)) {
            return collect_return_values(if_body, code, function_name, in_function);
        } else {
            // If there is an else branch, process it.
            if (ts_node_child_count(node) >= 5) {
                TSNode else_body = ts_node_child(node, 4);
                return collect_return_values(else_body, code, function_name, in_function);
            }
        }
    }
    
    // Handle function definitions.
    if (!strcmp(ts_node_type(node), "function_definition")) {
        TSNode function_declarator = ts_node_child(node, 1);
        TSNode identifier_node = ts_node_child(function_declarator, 0);
        std::string current_function_name = ts_node_string(identifier_node, code);
        if (current_function_name == function_name) {
            TSNode function_body = ts_node_child(node, 2);
            std::vector<std::string> return_values;
            uint32_t child_count = ts_node_child_count(function_body);
            for (uint32_t i = 0; i < child_count; ++i) {
                TSNode child = ts_node_child(function_body, i);
                std::vector<std::string> stm_returns = collect_return_values(child, code, function_name, true);
                return_values.insert(return_values.end(), stm_returns.begin(), stm_returns.end());
                // Stop traversing if an unconditional return is encountered.
                if (!stm_returns.empty()) {
                    break;
                }
            }
            return return_values;
        }
    }
    
    // Default: recursively check all child nodes.
    uint32_t child_count = ts_node_child_count(node);
    std::vector<std::string> return_values;
    for (uint32_t i = 0; i < child_count; ++i) {
        TSNode child = ts_node_child(node, i);
        std::vector<std::string> child_returns = collect_return_values(child, code, function_name, in_function);
        return_values.insert(return_values.end(), child_returns.begin(), child_returns.end());
        // Propagate an unconditional return if found.
        if (!child_returns.empty()) {
            break;
        }
    }
    return return_values;
}
