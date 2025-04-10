#pragma once //for simplicitly
#include <tree_sitter/api.h>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <set>
#include <algorithm>
#include <string.h>
extern "C" const TSLanguage *tree_sitter_cpp();
std::string ts_node_string(TSNode, const std::string&);