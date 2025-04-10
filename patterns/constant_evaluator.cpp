#include "common.h"
int evaluate_expression(TSNode node, const std::string &code, const std::unordered_map<std::string, int>& variables) {
    std::string type = ts_node_type(node);
    if (type == "number_literal") {
        uint32_t start = ts_node_start_byte(node);
        uint32_t end = ts_node_end_byte(node);
        std::string text = code.substr(start, end - start);
        return std::atoi(text.c_str());
    }
    if (type == "identifier") {
        uint32_t start = ts_node_start_byte(node);
        uint32_t end = ts_node_end_byte(node);
        std::string ident = code.substr(start, end - start);
        auto it = variables.find(ident);
        return (it != variables.end()) ? it->second : 0;
    }
    if (type == "binary_expression") {
        if (ts_node_child_count(node) < 3) return 0;
        TSNode left  = ts_node_child(node, 0);
        TSNode opNode = ts_node_child(node, 1);
        TSNode right = ts_node_child(node, 2);
        int leftVal = evaluate_expression(left, code, variables);
        int rightVal = evaluate_expression(right, code, variables);
        uint32_t opStart = ts_node_start_byte(opNode);
        uint32_t opEnd = ts_node_end_byte(opNode);
        std::string op = code.substr(opStart, opEnd - opStart);
        if (op == "+") return leftVal + rightVal;
        if (op == "-") return leftVal - rightVal;
        if (op == "*") return leftVal * rightVal;
        if (op == "/") return (rightVal != 0) ? leftVal / rightVal : 0;
    }
    if (type == "parenthesized_expression") {
        // Assume the inner expression is the second child.
        TSNode inner = ts_node_child(node, 1);
        return evaluate_expression(inner, code, variables);
    }
    return 0;
}

// Determines if the given condition evaluates to a non-zero value.
bool is_constant_true(TSNode condition, const std::string &code, const std::unordered_map<std::string, int>& variables) {
    int value = evaluate_expression(condition, code, variables);
    return value != 0;
}