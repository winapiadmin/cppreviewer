cmake_minimum_required(VERSION 3.10)
project(TreeSitterTools CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable debug info
set(CMAKE_BUILD_TYPE Debug)

# Include headers
include_directories(
    ${CMAKE_SOURCE_DIR}/tree-sitter
    #${CMAKE_SOURCE_DIR}/tree-sitter-cpp
)

# Compiler-specific flags
if(MSVC)
    add_compile_options(/Zi /W4)
    set(STATIC_LIBS
        ${CMAKE_SOURCE_DIR}/libtree-sitter-cpp.lib
        ${CMAKE_SOURCE_DIR}/libtree-sitter.lib
    )
else()
    add_compile_options(-g -ggdb -Wall -Wextra -pedantic)
    set(STATIC_LIBS
        ${CMAKE_SOURCE_DIR}/libtree-sitter-cpp.a
        ${CMAKE_SOURCE_DIR}/libtree-sitter.a
    )
endif()

# reviewer target
add_executable(reviewer parser.cpp)
target_link_libraries(reviewer PRIVATE ${STATIC_LIBS})
target_compile_definitions(reviewer PRIVATE TREE_SITTER_STATIC)

# xrefparser target
add_executable(xrefparser xref.cpp)
target_link_libraries(xrefparser PRIVATE ${STATIC_LIBS})
target_compile_definitions(xrefparser PRIVATE TREE_SITTER_STATIC)

# xrefparser target
add_executable(nodeprinter print.cpp)
target_link_libraries(nodeprinter PRIVATE ${STATIC_LIBS})
target_compile_definitions(nodeprinter PRIVATE TREE_SITTER_STATIC)
