#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "libraries/utilities/myassert.h"
#include "libraries/utilities/colors.h"
#include "libraries/stack/stack.h"
#include "libraries/onegin/onegin.h"
#include "libraries/utilities/utilities.h"
#include "akinator.h"

FILE *Global_logs_pointer  = stderr;
bool  Global_color_output  = true;

TYPE_ELEMENT_TREE POISON_TREE = "Неизвестно кто";

#ifdef DEBUG_OUTPUT_TREE_DUMP

    #define IF_ON_TREE_DUMP(...)   __VA_ARGS__

#else

    #define IF_ON_TREE_DUMP(...)

#endif

static ssize_t      verify_tree              (tree *tree_pointer, ssize_t line, const char *file, const char *func);
static tree_node   *new_tree_node();
static void delete_subtree(tree_node *tree_node_pointer);
// static ssize_t      tree_node_destructor     (tree *tree_pointer, debug_info_tree *info);


IF_ON_TREE_DUMP
(
    static void tree_dump               (tree *tree_pointer, ssize_t line, const char *file, const char *func);
    static void print_errors            (const tree *tree_pointer);
    static void print_debug_info        (const tree *tree_pointer, ssize_t line, const char *file, const char *func);
    static void generate_graph_of_tree  (tree *tree_pointer, ssize_t line, const char *file, const char *func);
    static void write_log_to_dot        (const tree *tree_pointer, FILE *dot_file, ssize_t line, const char *file, const char *func);
    static void write_subtree_to_dot    (const tree_node *tree_node_pointer, FILE *dot_file);
    static void generate_image          (const tree *tree_pointer, const char *name_dot_file, ssize_t number_graph);
)

#define VERIFY_TREE(tree_pointer) verify_tree(tree_pointer, __LINE__, __FILE__, __PRETTY_FUNCTION__)

#define CHECK_ERRORS(tree)                                          \
do {                                                                \
    if (((tree)->error_code = VERIFY_TREE(tree)) != TREE_NO_ERROR)  \
        return 0;                                                   \
} while(0)

tree *new_pointer_tree()
{
    struct tree *tree_pointer = (tree *) calloc(1, sizeof(tree));

    MYASSERT(tree_pointer  != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return NULL);

    return tree_pointer;
}

ssize_t tree_constructor(tree *tree_pointer, debug_info_tree *info)
{
    MYASSERT(tree_pointer  != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_TREE_IS_NULL);
    MYASSERT(info          != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_TREE_INFO_IS_NULL);

    tree_pointer->info = info;
    tree_pointer->size = 1;

    tree_pointer->root = new_tree_node();
    tree_pointer->root->left = new_tree_node();
    tree_pointer->root->left->data = "AAA";

    MYASSERT(tree_pointer->root != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return POINTER_TO_TREE_ROOT_IS_NULL);

    return (VERIFY_TREE(tree_pointer));
}

static tree_node *new_tree_node()
{
    tree_node *node_pointer = (tree_node *) calloc(1, sizeof(tree_node));

    MYASSERT(node_pointer != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return NULL);

    node_pointer->data = POISON_TREE;

    return node_pointer;
}

ssize_t tree_destructor(tree *tree_pointer)
{
    MYASSERT(tree_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_TREE_IS_NULL);
    MYASSERT(tree_pointer->root    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_TREE_ROOT_IS_NULL);
    MYASSERT(tree_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_TREE_INFO_IS_NULL);

    delete_subtree(tree_pointer->root);
    free(tree_pointer->info);

    tree_pointer->info = NULL;
    tree_pointer->root = NULL;

    tree_pointer->size = -1;

    free(tree_pointer);

    return TREE_NO_ERROR;
}

// void saving_tree_from_database(FILE *database_file, tree *tree_pointer)
// {
//     size_t size_file = determine_size(database_file);
//
//     char *database_buffer = (char *)calloc(size_file, sizeof(char));
//     MYASSERT(database_buffer != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
//
//     size_file = fread(database_buffer, sizeof(char), size_file, database_file);
//
//     for (size_t database_index = 0; database_index < size_file; database_index++)
//     {
//         if (isspace(database_buffer[database_index])) {
//             continue;
//         }
//
//         if (database_buffer[database_index] == '(') {
//             tree_pointer->root = new_tree_node();
//         }
//     }
//
//     free(database_buffer);
//
//     return (VERIFY_TREE(tree_pointer));
// }
//
// void saving_subtree_from_database(const char *database_buffer, tree_node *current_tree_node_pointer)
// {
//     MYASSERT(database_buffer != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
//
//     if (*database_buffer == '\0') {
//         return;
//     }
//
//     if ()
//
//     if (*database_buffer == '(')
// }

static void delete_subtree(tree_node *tree_node_pointer)
{
    if (!tree_node_pointer) {
        return;
    }

    delete_subtree(tree_node_pointer->left);
    delete_subtree(tree_node_pointer->right);

    tree_node_pointer->data  = POISON_TREE;
    tree_node_pointer->left  = NULL;
    tree_node_pointer->right = NULL;

    free(tree_node_pointer);
}

void print_node(tree_node *tree_node_pointer)
{
    if (!tree_node_pointer)
    {
        fprintf(stdout, "nil ");
        return;
    }

    fputc('(', stdout);

    fprintf(stdout, "%s ", tree_node_pointer->data);

    print_node(tree_node_pointer->left);
    print_node(tree_node_pointer->right);

    fputc(')', stdout);
}

static ssize_t verify_tree(tree *tree_pointer, ssize_t line, const char *file, const char *func)
{
    MYASSERT(tree_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_TREE_IS_NULL);
    MYASSERT(tree_pointer->root    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_TREE_ROOT_IS_NULL);
    MYASSERT(tree_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_TREE_INFO_IS_NULL);

    ssize_t error_code = TREE_NO_ERROR;

    #define SUMMARIZE_ERRORS_(condition, added_error)   \
    do {                                                \
        if((condition))                                 \
            error_code += added_error;                  \
    } while(0)

    #undef SUMMARIZE_ERRORS_

    tree_pointer->error_code = error_code;

    IF_ON_TREE_DUMP
    (
        if (error_code != TREE_NO_ERROR) {
            tree_dump(tree_pointer, line, file, func);
        }
    )

    tree_dump(tree_pointer, line, file, func);

    return tree_pointer->error_code;
}

IF_ON_TREE_DUMP
(
    static void tree_dump(tree *tree_pointer, ssize_t line, const char *file, const char *func)
    {
        MYASSERT(tree_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(tree_pointer->root    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(tree_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(Global_logs_pointer   != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(file                  != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(func                  != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

        print_errors(tree_pointer);

        print_debug_info(tree_pointer, line, file, func);

        generate_graph_of_tree(tree_pointer, line, file, func);
    }
)

IF_ON_TREE_DUMP
(
    static void print_debug_info(const tree *tree_pointer, ssize_t line, const char *file, const char *func)
    {
        MYASSERT(tree_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(tree_pointer->root    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(tree_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(Global_logs_pointer   != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(file                  != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(func                  != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

        COLOR_PRINT(MediumBlue, "tree[%p]\n", tree_pointer);

        COLOR_PRINT(BlueViolet, "\"%s\"from %s(%ld) %s\n", tree_pointer->info->name, tree_pointer->info->file, tree_pointer->info->line, tree_pointer->info->func);

        COLOR_PRINT(DarkMagenta, "called from %s(%ld) %s\n", file, line, func);

        fprintf(Global_logs_pointer,  "\nroot = ");
        COLOR_PRINT(Orange, "%p\n", tree_pointer->root);

        fprintf(Global_logs_pointer, "size = ");
        COLOR_PRINT(Crimson, "%ld\n", tree_pointer->size);
    }
)

#define GET_ERRORS_(error)                                                           \
do {                                                                                 \
    if(tree_pointer->error_code & error)                                             \
        COLOR_PRINT(Red, "Errors: %s\n", #error);                                    \
} while(0)

IF_ON_TREE_DUMP
(
    static void print_errors(const tree *tree_pointer)
    {
        MYASSERT(tree_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(tree_pointer->root    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(tree_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(Global_logs_pointer   != NULL, NULL_POINTER_PASSED_TO_FUNC, return);


    }
)

#undef GET_ERRORS_

IF_ON_TREE_DUMP
(
    static void generate_graph_of_tree(tree *tree_pointer, ssize_t line, const char *file, const char *func)
    {
        MYASSERT(tree_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(tree_pointer->root    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(tree_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(Global_logs_pointer   != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(file                  != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(func                  != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

        const char  *NAME_DOT_FILE   = "tree.dot";
        static ssize_t number_graph = 0;

        ++number_graph;

        FILE *dot_file = check_isopen(NAME_DOT_FILE, "w");

        write_log_to_dot(tree_pointer, dot_file, line, file, func);

        MYASSERT(check_isclose(dot_file), COULD_NOT_CLOSE_THE_FILE , return);

        generate_image(tree_pointer, NAME_DOT_FILE, number_graph);

        fprintf(Global_logs_pointer, "<img src = graph/tree_%ld.png height= 75%% width = 75%%>\n\n", number_graph);
    }
)

IF_ON_TREE_DUMP
(
    static void write_log_to_dot(const tree *tree_pointer, FILE *dot_file, ssize_t line, const char *file, const char *func)
    {
        MYASSERT(tree_pointer           != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(tree_pointer->root     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(tree_pointer->info     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(dot_file               != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(file                   != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(func                   != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

        fprintf(dot_file,   "digraph Tree {\n"
                            "\trankdir = TB;\n"
	                        "\tnode [shape = record];\n"
                            "\tsplines=ortho;\n");


        fprintf(dot_file,  "\tsubgraph cluster0 {\n"
                           "\t\tlabel = \"called  from:    %s(%ld)  %s\";\n", file, line, func);

        write_subtree_to_dot(tree_pointer->root, dot_file);

        fprintf(dot_file,   "\n\n\t\tInfo[shape = Mrecord, label = \"size = %ld \"];\n"
                            "\t}\n"
                            "}\n",
                tree_pointer->size);
    }
)

IF_ON_TREE_DUMP
(
    static void write_subtree_to_dot(const tree_node *tree_node_pointer, FILE *dot_file)
    {
        MYASSERT(tree_node_pointer      != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(dot_file               != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

        fprintf(dot_file,  "\t\tnode_%p  [shape = Mrecord, style = filled, fillcolor = \"#ba7fa2\", label = \"{" FORMAT_SPECIFIERS_TREE "| {<left> да | <right> нет}} \"];\n", tree_node_pointer, tree_node_pointer->data);

        if (tree_node_pointer->left != NULL)
        {
            write_subtree_to_dot(tree_node_pointer->left, dot_file);

            fprintf(dot_file, "\n\tnode_%p:<left> -> node_%p \t[color = blue];\n", tree_node_pointer, tree_node_pointer->left);
        }

        if (tree_node_pointer->right != NULL)
        {
            write_subtree_to_dot(tree_node_pointer->right, dot_file);

            fprintf(dot_file, "\n\tnode_%p:<right> -> node_%p \t[color = blue];\n", tree_node_pointer, tree_node_pointer->right);
        }
    }
)

IF_ON_TREE_DUMP
(
    static void generate_image(const tree *tree_pointer, const char *name_dot_file, ssize_t number_graph)
    {
        MYASSERT(tree_pointer           != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(tree_pointer->root     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(tree_pointer->info     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(name_dot_file          != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

        const size_t SIZE_NAME_GRAPH = 50;

        char image_generation_command[SIZE_NAME_GRAPH] = "";

        snprintf(image_generation_command, SIZE_NAME_GRAPH, "dot %s -T png -o graph/tree_%ld.png", name_dot_file, number_graph);

        MYASSERT(!system(image_generation_command), SYSTEM_ERROR, return);
    }
)
