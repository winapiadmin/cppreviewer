#include "common.h"
#include <iomanip>
std::string find_identifier(TSNode node, const std::string& code) {
    if (std::string(ts_node_type(node)) == "identifier")
        return ts_node_string(node, code);
    uint32_t count = ts_node_child_count(node);
    for (uint32_t i = 0; i < count; ++i) {
        TSNode child = ts_node_child(node, i);
        std::string name = find_identifier(child, code);
        if (!name.empty())
            return name;
    }
    return "";
}

// Struct to hold collected function information
struct FunctionInfo {
    std::string name;
    std::string declaration;
};
std::vector<FunctionInfo> functions;
// Recursively traverses the syntax tree to collect functions.
// It checks several node types: "template_declaration", "function_definition",
// "declaration", and "function_declaration" to extract the function name
// and its associated declaration text.
void collect_functions(TSNode node, const std::string& code) {
    std::string node_type = ts_node_type(node);
    if (node_type=="field_declaration"){
        if (ts_node_child_count(node) <=2) return;
        TSNode primitive_type = ts_node_child(node, 0),
               declarator = ts_node_child(node, 1);
        if (!strcmp(ts_node_type(primitive_type), "primitive_type") &&
            !strcmp(ts_node_type(declarator), "function_declarator")) {
            std::string function_name = ts_node_string(ts_node_child(declarator, 0), code);
            std::string declaration_text= ts_node_string(node, code);
            auto it = std::find_if(functions.begin(), functions.end(), [&function_name, &declaration_text](const FunctionInfo& info) {
                return info.name == function_name&& info.declaration == declaration_text;//yes of course templates
            });
            if (it == functions.end()) {
                FunctionInfo info;
                info.name = function_name;
                info.declaration = declaration_text;
                functions.push_back(info);
            }
        }
    }
    if ((node_type == "function_definition" || node_type == "function_declaration")) { // found a declarator & likely a function
        std::string function_name = find_identifier(node, code);
        std::string declaration_text;
        if (node_type == "function_definition") {
            TSNode primitive_type = ts_node_child(node, 0);
            if (!strcmp(ts_node_type(primitive_type), "function_declarator")) {
                if (ts_node_child_count(primitive_type) > 0&&
                    !strcmp(ts_node_type(ts_node_child(primitive_type, 0)), "destructor_name"))
                        function_name=ts_node_string(ts_node_child(primitive_type, 0), code);
                size_t start_byte = ts_node_start_byte(primitive_type);
                size_t end_byte = ts_node_end_byte(primitive_type);
                declaration_text = code.substr(start_byte, end_byte - start_byte) + ";";
                goto fine;
            }
            else{
                TSNode func_decl = ts_node_child(node, 1);
                if (!strcmp(ts_node_type(func_decl), "function_declarator")) {
                    if (ts_node_child_count(func_decl) > 1 &&
                        !strcmp(ts_node_type(ts_node_child(func_decl, 0)), "field_identifier"))
                        function_name = ts_node_string(ts_node_child(func_decl, 0), code);
                }
                size_t start_byte = ts_node_start_byte(primitive_type);
                size_t end_byte = ts_node_end_byte(func_decl);
                declaration_text = code.substr(start_byte, end_byte - start_byte) + ";";
            }
        }
        fine:
        auto it = std::find_if(functions.begin(), functions.end(), [&function_name, &declaration_text](const FunctionInfo& info) {
            return info.name == function_name&& info.declaration == declaration_text;//yes of course templates
        });
        if (it == functions.end()) {
            FunctionInfo info;
            info.name = function_name;
            info.declaration = declaration_text;
            functions.push_back(info);
        }
    }
    
    // Recurse into children
    uint32_t total_children = ts_node_child_count(node);
    for (uint32_t i = 0; i < total_children; ++i) {
        TSNode child = ts_node_child(node, i);
        collect_functions(child, code);
    }
}
// Print the collected functions in a table-like format
void print_function_table() {
    std::cout << "-------------------------------------------------------------\n";
    std::cout << std::left << std::setw(30) << "Function Name" << std::setw(60) << "Declaration" << "\n";
    std::cout << "-------------------------------------------------------------\n";
    for (const auto& func : functions) {
        std::cout << std::left << std::setw(30) << func.name << std::setw(60) << func.declaration << "\n";
    }
    std::cout << "-------------------------------------------------------------\n";
}
