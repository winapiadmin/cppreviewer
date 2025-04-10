#pragma once
#include "common.h"
#include <cstdlib>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <cctype>

// Evaluates a basic arithmetic expression from the AST.
// Only supports number literals, identifiers (looked up in variables),
// binary expressions with +, -, *, /, and parenthesized expressions.
int evaluate_expression(TSNode, const std::string&, const std::unordered_map<std::string, int>&);
bool is_constant_true(TSNode, const std::string&, const std::unordered_map<std::string, int>&);