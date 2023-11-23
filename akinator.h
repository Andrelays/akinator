#ifndef TREE_H_INCLUDED
#define TREE_H_INCLUDED

#include <stdlib.h>

extern FILE *Global_logs_pointer;
extern bool  Global_color_output;

#define TREE_CONSTRUCTOR(tree_pointer)                                                          \
do {                                                                                            \
    struct debug_info_tree *info = (debug_info_tree *) calloc(1, sizeof(debug_info_tree));      \
                                                                                                \
    info->line = __LINE__;                                                                      \
    info->name = #tree_pointer;                                                                 \
    info->file = __FILE__;                                                                      \
    info->func = __PRETTY_FUNCTION__;                                                           \
                                                                                                \
    tree_constructor(tree_pointer, info);                                                       \
} while(0)

#define FORMAT_SPECIFIERS_TREE   "%s"
typedef const char* TYPE_ELEMENT_TREE;

enum errors_code_tree {
    TREE_NO_ERROR                           = 0,
    POINTER_TO_TREE_IS_NULL                 = 1,
    POINTER_TO_TREE_ROOT_IS_NULL            = 1 <<  1,
    POINTER_TO_TREE_INFO_IS_NULL            = 1 <<  2,
    POINTER_TO_TREE_NODE_IS_NULL            = 1 <<  3
};

struct debug_info_tree {
    ssize_t      line;
    const char  *name;
    const char  *file;
    const char  *func;
};

struct tree_node {
    TYPE_ELEMENT_TREE        data;
    tree_node               *left;
    tree_node               *right;
};

struct tree {
    tree_node               *root;
    ssize_t                  size;
    ssize_t                  error_code;
    struct debug_info_tree  *info;
};

tree *new_pointer_tree();

ssize_t           tree_constructor  (tree *tree_pointer, debug_info_tree *info);
ssize_t           tree_destructor   (tree *tree_pointer);
void              print_node        (tree_node *tree_node_pointer);



#endif //TREE_H_INCLUDED
