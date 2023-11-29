#ifndef TREE_H_INCLUDED
#define TREE_H_INCLUDED

#include <stdlib.h>

#ifdef DEBUG_OUTPUT_TREE_DUMP

    #define IF_ON_TREE_DUMP(...)   __VA_ARGS__

#else

    #define IF_ON_TREE_DUMP(...)

#endif

extern FILE *Global_logs_pointer_tree;
extern bool  Global_color_output_tree;

const size_t MAX_SIZE_NAME_ELEMENT  = 100;

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
typedef char TYPE_ELEMENT_TREE;

enum errors_code_tree {
    TREE_NO_ERROR                           = 0,
    POINTER_TO_TREE_IS_NULL                 = 1,
    POINTER_TO_TREE_ROOT_IS_NULL            = 1 <<  1,
    POINTER_TO_TREE_INFO_IS_NULL            = 1 <<  2,
    POINTER_TO_TREE_NODE_IS_NULL            = 1 <<  3,
    STORAGE_LOCATE_NODE_IS_NULL             = 1 <<  4,
    NAME_ELEMENT_IS_NULL                    = 1 <<  5,
    DISTINCTIVE_PROPERTY_IS_NULL            = 1 <<  6,
};

struct debug_info_tree {
    ssize_t      line;
    const char  *name;
    const char  *file;
    const char  *func;
};

struct tree_node {
    TYPE_ELEMENT_TREE        data[MAX_SIZE_NAME_ELEMENT];
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

bool check_answer_to_question();

IF_ON_TREE_DUMP(void tree_dump(tree *tree_pointer, ssize_t line, const char *file, const char *func));
int print_and_say(const char *fmt, ...);
ssize_t tree_constructor(tree *tree_pointer, debug_info_tree *info);
ssize_t tree_destructor(tree *tree_pointer);
void print_node(tree_node *tree_node_pointer, FILE *file_output);
tree_node **guess_answer(tree *tree_pointer);
ssize_t inserting_new_element(tree *tree_pointer, tree_node **storage_location_node, const char *distinctive_property, const char *name_element);
void give_definition_element(tree *tree_pointer, const char *name_desired_element);
void flush_buffer();
void find_difference_between_elements(tree *tree_pointer, const char *name_compared_element_1, const char *name_compared_element_2);
void saving_tree_from_database(FILE *database_file, tree *tree_pointer);
void fgets_whithout_newline(char *str, int num_chars, FILE *stream);

#endif //TREE_H_INCLUDED
