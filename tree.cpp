#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "libraries/utilities/myassert.h"
#include "libraries/utilities/colors.h"
#include "libraries/stack/stack.h"
#include "libraries/onegin/onegin.h"
#include "libraries/utilities/utilities.h"
#include "akinator.h"

const int LEFT  = 1;
const int RIGHT = 0;

FILE *Global_logs_pointer_tree  = stderr;
bool  Global_color_output_tree  = true;

const char *POISON_TREE = "Неизвестно кто";

#ifdef DEBUG_OUTPUT_TREE_DUMP

    #define IF_ON_TREE_DUMP(...)   __VA_ARGS__

#else

    #define IF_ON_TREE_DUMP(...)

#endif

static void printing_element_definition (stack *stk, tree_node *current_node);
static ssize_t verify_tree (tree *tree_pointer, ssize_t line, const char *file, const char *func);
static tree_node *new_tree_node();
static void delete_subtree(tree_node *tree_node_pointer);
static bool find_path_to_elememnt(tree *tree_pointer, stack *stk, const char *name_desired_element);
static bool rec_element_search(stack *stk, tree_node *current_node, const char *name_desired_element);
static void saving_node_from_database(const char *database_buffer, tree_node *current_tree_node);

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
        return (tree)->error_code;                                  \
} while(0)

tree *new_pointer_tree()
{
    struct tree *tree_pointer = (tree *) calloc(1, sizeof(tree));

    MYASSERT(tree_pointer  != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return NULL);

    return tree_pointer;
}

void fgets_whithout_newline(char *str, int num_chars, FILE *stream)
{
    MYASSERT(str    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(stream != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

    MYASSERT(fgets(str, num_chars, stream), SYSTEM_ERROR, return);

    str[strcspn(str, "\r\n")] = 0;
}

ssize_t tree_constructor(tree *tree_pointer, debug_info_tree *info)
{
    MYASSERT(tree_pointer  != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_TREE_IS_NULL);
    MYASSERT(info          != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_TREE_INFO_IS_NULL);

    tree_pointer->info = info;
    tree_pointer->size = 1;

    tree_pointer->root = new_tree_node();

    MYASSERT(tree_pointer->root != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return POINTER_TO_TREE_ROOT_IS_NULL);

    return (VERIFY_TREE(tree_pointer));
}

static tree_node *new_tree_node()
{
    tree_node *node_pointer = (tree_node *) calloc(1, sizeof(tree_node));

    MYASSERT(node_pointer != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return NULL);

    strncpy(node_pointer->data, POISON_TREE, MAX_SIZE_NAME_ELEMENT);

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

static void delete_subtree(tree_node *tree_node_pointer)
{
    if (!tree_node_pointer) {
        return;
    }

    delete_subtree(tree_node_pointer->left);
    delete_subtree(tree_node_pointer->right);

    strncpy(tree_node_pointer->data, POISON_TREE, MAX_SIZE_NAME_ELEMENT);
    tree_node_pointer->left  = NULL;
    tree_node_pointer->right = NULL;

    free(tree_node_pointer);
}

tree_node **guess_answer(tree *tree_pointer)
{
    MYASSERT(tree_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(tree_pointer->root    != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(tree_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);

    tree_node **storage_location_node = &(tree_pointer->root);

    while ((*storage_location_node)->left != NULL)
    {
        printf("Это %s?\n", (*storage_location_node)->data);

        if (check_answer_to_question()) {
            storage_location_node = &((*storage_location_node)->left);
        }

        else {
            storage_location_node = &((*storage_location_node)->right);
        }
    }

    return storage_location_node;
}

void flush_buffer()
{
    int symbol = 0;

    while ((symbol = getchar()) != '\n' && symbol != EOF) {
        continue;
    }
}

bool check_answer_to_question()
{
    const size_t SIZE_ANSWER = 100;
    char answer[SIZE_ANSWER] = "";

    while (true)
    {
        memset(answer, 0, SIZE_ANSWER);

        fgets_whithout_newline(answer, MAX_SIZE_NAME_ELEMENT, stdin);

        if (strcmp(answer, "да") == 0) {
            return true;
        }

        if (strcmp(answer, "нет") == 0) {
            return false;
        }

        printf("Ошибка! Был дан неправильный ответ. Введите его ещё раз (принимается только \"да\" или \"нет\"):\n");
    }
}


ssize_t inserting_new_element(tree *tree_pointer, tree_node **storage_location_node, const char *distinctive_property, const char *name_element)
{
    MYASSERT(tree_pointer            != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_TREE_IS_NULL);
    MYASSERT(tree_pointer->root      != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_TREE_ROOT_IS_NULL);
    MYASSERT(tree_pointer->info      != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_TREE_INFO_IS_NULL);
    MYASSERT(storage_location_node   != NULL, NULL_POINTER_PASSED_TO_FUNC, return STORAGE_LOCATE_NODE_IS_NULL);
    MYASSERT(*storage_location_node  != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_TREE_NODE_IS_NULL);
    MYASSERT(distinctive_property    != NULL, NULL_POINTER_PASSED_TO_FUNC, return NAME_ELEMENT_IS_NULL);
    MYASSERT(name_element            != NULL, NULL_POINTER_PASSED_TO_FUNC, return DISTINCTIVE_PROPERTY_IS_NULL);

    CHECK_ERRORS(tree_pointer);

    printf("%s\n", (*storage_location_node)->data);

    tree_node *replaceable_node          = *storage_location_node;
    tree_node *new_element_node          = new_tree_node();
    tree_node *distinctive_property_node = new_tree_node();

    MYASSERT(new_element_node          != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return POINTER_TO_TREE_NODE_IS_NULL);
    MYASSERT(distinctive_property_node != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return POINTER_TO_TREE_NODE_IS_NULL);

    strncpy(distinctive_property_node->data, distinctive_property, MAX_SIZE_NAME_ELEMENT);
    strncpy(new_element_node->data,          name_element,         MAX_SIZE_NAME_ELEMENT);

    distinctive_property_node->left  = new_element_node;
    distinctive_property_node->right = replaceable_node;

    *storage_location_node = distinctive_property_node;

    printf("%s\n", tree_pointer->root->data);

    tree_pointer->size += 2;

    return VERIFY_TREE(tree_pointer);
}

void find_difference_between_elements(tree *tree_pointer, const char *name_compared_element_1, const char *name_compared_element_2)
{
    MYASSERT(tree_pointer               != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(tree_pointer->root         != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(tree_pointer->info         != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(name_compared_element_1    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(name_compared_element_2    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

    int pop_value_1 = 0, pop_value_2 = 0;

    stack *stk_element_1 = get_pointer_stack();
    stack *stk_element_2 = get_pointer_stack();

    STACK_CONSTRUCTOR(stk_element_1);
    STACK_CONSTRUCTOR(stk_element_2);

    if (!find_path_to_elememnt(tree_pointer, stk_element_1, name_compared_element_1) || !find_path_to_elememnt(tree_pointer, stk_element_2, name_compared_element_2))
    {
        stack_destructor(stk_element_1);
        stack_destructor(stk_element_2);

        return;
    }

    tree_node *current_node_element_1 = tree_pointer->root;
    tree_node *current_node_element_2 = tree_pointer->root;

    printf("%s и %s вместе - ", name_compared_element_1, name_compared_element_2);

    while (stk_element_1->size > 0 && stk_element_2->size > 0)
    {
        pop(stk_element_1, &pop_value_1);
        pop(stk_element_2, &pop_value_2);

        if (pop_value_1 != pop_value_2)
        {
            push(stk_element_1, pop_value_1);
            push(stk_element_2, pop_value_2);

            break;
        }

        if (pop_value_1 == LEFT)
        {
            printf("%s ", current_node_element_1->data);

            current_node_element_1 = current_node_element_1->left;
            current_node_element_2 = current_node_element_2->left;
        }

        else
        {
            printf("НЕ %s ", current_node_element_1->data);

            current_node_element_1 = current_node_element_1->right;
            current_node_element_2 = current_node_element_2->left;
        }
    }

    printf("\nНо %s - ", name_compared_element_1);
    printing_element_definition(stk_element_1, current_node_element_1);

    printf("А %s - ", name_compared_element_2);
    printing_element_definition(stk_element_2, current_node_element_2);

    stack_destructor(stk_element_1);
    stack_destructor(stk_element_2);
}

void define_element(tree *tree_pointer, const char *name_desired_element)
{
    MYASSERT(tree_pointer            != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(tree_pointer->root      != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(tree_pointer->info      != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(name_desired_element    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

    stack *stk = get_pointer_stack();
    STACK_CONSTRUCTOR(stk);

    if (!find_path_to_elememnt(tree_pointer, stk, name_desired_element))
    {
        stack_destructor(stk);
        return;
    }

    printf("%s - ", name_desired_element);

    printing_element_definition(stk, tree_pointer->root);

    stack_destructor(stk);
}

static void printing_element_definition(stack *stk, tree_node *current_node)
{
    int pop_value = 0;

    while (stk->size > 0)
    {
        pop(stk, &pop_value);

        if (pop_value == LEFT) {
            printf("%s ", current_node->data);
            current_node = current_node->left;
        }

        else {
            printf("НЕ %s ", current_node->data);
            current_node = current_node->right;
        }
    }

    putchar('\n');
}

static bool find_path_to_elememnt(tree *tree_pointer, stack *stk, const char *name_desired_element)
{
    MYASSERT(tree_pointer            != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
    MYASSERT(tree_pointer->root      != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
    MYASSERT(tree_pointer->info      != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
    MYASSERT(name_desired_element    != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
    MYASSERT(stk                     != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);


    if (!rec_element_search(stk, tree_pointer->root, name_desired_element))
    {
        printf("Элемент \"%s\" не был найден!\n", name_desired_element);
        return false;
    }

    return true;
}

static bool rec_element_search(stack *stk, tree_node *current_node, const char *name_desired_element)
{
    MYASSERT(name_desired_element    != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
    MYASSERT(stk                     != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);

    if(!current_node) {
        return false;
    }

    if (strcmp(current_node->data, name_desired_element) == 0) {
        return true;
    }

    if (rec_element_search(stk, current_node->left, name_desired_element)) {
        push(stk, LEFT);
        return true;
    }

    if (rec_element_search(stk, current_node->right, name_desired_element)) {
        push(stk, RIGHT);
        return true;
    }

    return false;
}

// void saving_tree_from_database(FILE *database_file, tree *tree_pointer)
// {
//     size_t size_file = determine_size(database_file);
//
//     char *database_buffer = (char *)calloc(size_file + 1, sizeof(char));
//
//     MYASSERT(database_buffer != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
//
//     size_file = fread(database_buffer, sizeof(char), size_file, database_file);
//     database_buffer[size_file] = '\0';
//
//     saving_node_from_database(database_buffer, tree_pointer->root);
//
//     free(database_buffer);
//
//     VERIFY_TREE(tree_pointer);
//
//     return;
// }
//
// static void saving_node_from_database(const char *database_buffer, tree_node *current_tree_node)
// {
//     MYASSERT(database_buffer != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
//
//     char data_element[MAX_SIZE_NAME_ELEMENT] = "";
//
//     while(*database_buffer != '\0')
//     {
//         database_buffer = skip_spaces(database_buffer);
//
//         if (*database_buffer == '\0') {
//             return;
//         }
//
//         if (*database_buffer == '(')
//         {
//             ++database_buffer;
//
//             tree_node *new_element_node = new_tree_node();
//             saving_node_from_database(database_buffer, new_element_node);
//
//             return;
//         }
//
//         if (strncmp(database_buffer, "nil", sizeof("nil") - 1) == 0) {
//             database_buffer += 3;
//         }
//
//         else {
//             sscanf(database_buffer, "%s", data_element);
//
//             strncpy(current_tree_node->data, data_element, MAX_SIZE_NAME_ELEMENT);
//         }
//     }
// }

void print_node(tree_node *tree_node_pointer, FILE *file_output)
{
    MYASSERT(file_output != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

    if (!tree_node_pointer)
    {
        fprintf(file_output, "nil ");
        return;
    }

    fprintf(file_output, "( %s ", tree_node_pointer->data);

    print_node(tree_node_pointer->left,  file_output);
    print_node(tree_node_pointer->right, file_output);

    fputc(')', file_output);
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
        MYASSERT(tree_pointer               != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(tree_pointer->root         != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(tree_pointer->info         != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(Global_logs_pointer_tree   != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(file                       != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(func                       != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

        print_errors(tree_pointer);

        print_debug_info(tree_pointer, line, file, func);

        generate_graph_of_tree(tree_pointer, line, file, func);
    }
)

IF_ON_TREE_DUMP
(
    static void print_debug_info(const tree *tree_pointer, ssize_t line, const char *file, const char *func)
    {
        MYASSERT(tree_pointer               != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(tree_pointer->root         != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(tree_pointer->info         != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(Global_logs_pointer_tree   != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(file                       != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(func                       != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

        COLOR_PRINT(MediumBlue, "tree[%p]\n", tree_pointer);

        COLOR_PRINT(BlueViolet, "\"%s\"from %s(%ld) %s\n", tree_pointer->info->name, tree_pointer->info->file, tree_pointer->info->line, tree_pointer->info->func);

        COLOR_PRINT(DarkMagenta, "called from %s(%ld) %s\n", file, line, func);

        fprintf(Global_logs_pointer_tree,  "\nroot = ");
        COLOR_PRINT(Orange, "%p\n", tree_pointer->root);

        fprintf(Global_logs_pointer_tree, "size = ");
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
        MYASSERT(Global_logs_pointer_tree   != NULL, NULL_POINTER_PASSED_TO_FUNC, return);


    }
)

#undef GET_ERRORS_

IF_ON_TREE_DUMP
(
    static void generate_graph_of_tree(tree *tree_pointer, ssize_t line, const char *file, const char *func)
    {
        MYASSERT(tree_pointer               != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(tree_pointer->root         != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(tree_pointer->info         != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(Global_logs_pointer_tree   != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(file                       != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(func                       != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

        const char  *NAME_DOT_FILE   = "tree.dot";
        static ssize_t number_graph = 0;

        ++number_graph;

        FILE *dot_file = check_isopen(NAME_DOT_FILE, "w");

        write_log_to_dot(tree_pointer, dot_file, line, file, func);

        MYASSERT(check_isclose(dot_file), COULD_NOT_CLOSE_THE_FILE , return);

        generate_image(tree_pointer, NAME_DOT_FILE, number_graph);

        fprintf(Global_logs_pointer_tree, "<img src = graph/tree_%ld.png height= 75%% width = 75%%>\n\n", number_graph);
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
