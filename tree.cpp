#include <stdio.h>
#include <string.h>
#include <stdarg.h>
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

static void printing_element_definition (stack *stk, tree_node *current_node);
static ssize_t verify_tree (tree *tree_pointer, ssize_t line, const char *file, const char *func);
static tree_node *new_tree_node();
static void delete_subtree(tree_node *tree_node_pointer);
static bool find_path_to_element(tree *tree_pointer, stack *stk, const char *name_desired_element);
static bool rec_element_search(stack *stk, tree_node *current_node, const char *name_desired_element);
static tree_node *saving_node_from_database(char *database_buffer);

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
        print_and_say("Это %s?\n", (*storage_location_node)->data);

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

        fgets_whithout_newline(answer, SIZE_ANSWER, stdin);

        if (strcmp(answer, "да") == 0) {
            return true;
        }

        if (strcmp(answer, "нет") == 0) {
            return false;
        }

        print_and_say("Ошибка! Был дан неправильный ответ. Введите его ещё раз (принимается только \"да\" или \"нет\", возражения не принимаются):\n");
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

    if (!find_path_to_element(tree_pointer, stk_element_1, name_compared_element_1) || !find_path_to_element(tree_pointer, stk_element_2, name_compared_element_2))
    {
        stack_destructor(stk_element_1);
        stack_destructor(stk_element_2);

        return;
    }

    tree_node *current_node_element_1 = tree_pointer->root;
    tree_node *current_node_element_2 = tree_pointer->root;

    print_and_say("%s и %s вместе - ", name_compared_element_1, name_compared_element_2);

    while (stk_element_1->size > 0 && stk_element_2->size > 0)
    {
        pop(stk_element_1, &pop_value_1);
        pop(stk_element_2, &pop_value_2);

        if (pop_value_1 != pop_value_2)
        {
            push(stk_element_1, pop_value_1);
            print_and_say("\nНо %s -", name_compared_element_1);
            printing_element_definition(stk_element_1, current_node_element_1);

            push(stk_element_2, pop_value_2);
            print_and_say("А %s - ", name_compared_element_2);
            printing_element_definition(stk_element_2, current_node_element_2);

            break;
        }

        if (pop_value_1 == LEFT)
        {
            print_and_say("%s ", current_node_element_1->data);

            current_node_element_1 = current_node_element_1->left;
            current_node_element_2 = current_node_element_2->left;
        }

        else
        {
            print_and_say("НЕ %s ", current_node_element_1->data);

            current_node_element_1 = current_node_element_1->right;
            current_node_element_2 = current_node_element_2->right;
        }
    }

    stack_destructor(stk_element_1);
    stack_destructor(stk_element_2);
}

void give_definition_element(tree *tree_pointer, const char *name_desired_element)
{
    MYASSERT(tree_pointer            != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(tree_pointer->root      != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(tree_pointer->info      != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(name_desired_element    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

    stack *stk = get_pointer_stack();
    STACK_CONSTRUCTOR(stk);

    if (!find_path_to_element(tree_pointer, stk, name_desired_element))
    {
        stack_destructor(stk);
        return;
    }

    print_and_say("%s - ", name_desired_element);

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
            print_and_say("%s ", current_node->data);
            current_node = current_node->left;
        }

        else {
            print_and_say("НЕ %s ", current_node->data);
            current_node = current_node->right;
        }
    }

    putchar('\n');
}

static bool find_path_to_element(tree *tree_pointer, stack *stk, const char *name_desired_element)
{
    MYASSERT(tree_pointer            != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
    MYASSERT(tree_pointer->root      != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
    MYASSERT(tree_pointer->info      != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
    MYASSERT(name_desired_element    != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
    MYASSERT(stk                     != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);

    if (!rec_element_search(stk, tree_pointer->root, name_desired_element))
    {
        print_and_say("Сам ты \"%s\"\n", name_desired_element);
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

    if (strcmp(current_node->data, name_desired_element) == 0 && !current_node->left && !current_node->right) {
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

void saving_tree_from_database(FILE *database_file, tree *tree_pointer)
{
    size_t size_file = determine_size(database_file);

    char *database_buffer = (char *)calloc(size_file + 1, sizeof(char));

    MYASSERT(database_buffer != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

    size_file = fread(database_buffer, sizeof(char), size_file, database_file);
    database_buffer[size_file] = '\0';

    delete_subtree(tree_pointer->root);

    tree_pointer->root = saving_node_from_database(database_buffer);

    // free(database_buffer);

    VERIFY_TREE(tree_pointer);

    return;
}

static tree_node *saving_node_from_database(char *database_buffer)
{
    static char *current_buffer = NULL;

    if (database_buffer == NULL) {
        current_buffer = strtok(NULL, " \n\r\t");
    }

    else {
        database_buffer = strtok(database_buffer, " \n\r\t");
        current_buffer  = database_buffer;
    }

    if (current_buffer == NULL) {
        print_and_say("Ошибка при считывании дерева из файла\n");
        return NULL;
    }

//     else {
//         strcat(current_tree_node->data, current_buffer);
//
//         current_buffer = strtok(NULL, " \n\r\t");
//
//         if (current_buffer == NULL) {
//             print_and_say("Ошибка при считывании дерева из файла\n");
//             return NULL;
//         }
//     }

    if (*current_buffer == '(')
    {
        current_buffer = strtok(NULL, " \n\r\t");

        if (current_buffer == NULL) {
            print_and_say("Ошибка при считывании дерева из файла\n");
            return NULL;
        }

        tree_node *current_tree_node = new_tree_node();

        strncpy(current_tree_node->data, current_buffer, MAX_SIZE_NAME_ELEMENT);

        current_tree_node->left  = saving_node_from_database(NULL);
        current_tree_node->right = saving_node_from_database(NULL);

        return current_tree_node;
    }

    if (strncmp(current_buffer, "nil", sizeof("nil") - 1) == 0) {
        return NULL;
    }

    if (*current_buffer == ')') {
        return saving_node_from_database(NULL);
    }

    return NULL;
}


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

    fprintf(file_output, ") ");
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

#ifdef ON_MODE_SAY
    int print_and_say(const char *fmt, ...)
    {
        va_list args_1, args_2;
        va_start(args_1, fmt);
        va_start(args_2, fmt);

        FILE *file_output = popen("festival --language russian --tts", "w");

        int ret = vprintf(fmt, args_2);
        vfprintf(file_output, fmt, args_1);

        pclose(file_output);

        va_end(args_1);
        va_end(args_2);

        return ret;
    }

#else
    int print_and_say(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);

        int ret = vprintf(fmt, args);

        va_end(args);

        return ret;
    }

#endif
