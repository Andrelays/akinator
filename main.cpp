#include <string.h>
#include <stdio.h>
#include "libraries/utilities/myassert.h"
#include "libraries/utilities/utilities.h"
#include "akinator.h"

int main(int argc, const char *argv[])
{
    const int    CORRECT_NUMBER_OF_ARGC = 4;

    if(!check_argc(argc, CORRECT_NUMBER_OF_ARGC)) {
        return INCORRECT_NUMBER_OF_ARGC;
    }

    const char *file_name_logs   = argv[1];
    const char *file_name_input  = argv[2];
    const char *file_name_output = argv[3];


    Global_logs_pointer_tree = check_isopen_html(file_name_logs, "w");
    FILE *file_input         = check_isopen(file_name_input,     "r");
    FILE *file_output        = check_isopen(file_name_output,    "w");


    MYASSERT(Global_logs_pointer_tree != NULL, COULD_NOT_OPEN_THE_FILE , return COULD_NOT_OPEN_THE_FILE);
    MYASSERT(file_input               != NULL, COULD_NOT_OPEN_THE_FILE , return COULD_NOT_OPEN_THE_FILE);
    MYASSERT(file_output              != NULL, COULD_NOT_OPEN_THE_FILE , return COULD_NOT_OPEN_THE_FILE);


    tree *tree_pointer = new_pointer_tree();

    TREE_CONSTRUCTOR(tree_pointer);

    char name_of_element_1[MAX_SIZE_NAME_ELEMENT]      = "";
    char name_of_element_2[MAX_SIZE_NAME_ELEMENT]      = "";
    char distinctive_property[MAX_SIZE_NAME_ELEMENT]   = "";

    bool user_wants_terminate_programm = false;
    tree_node **storage_location_node  = NULL;

    printf("Хотите ли вы скачать указанную базу данных?\n");

    if (check_answer_to_question()) {
        saving_tree_from_database(file_input, tree_pointer);
    }

    do {
        memset(name_of_element_1,    0, MAX_SIZE_NAME_ELEMENT);
        memset(name_of_element_2,    0, MAX_SIZE_NAME_ELEMENT);
        memset(distinctive_property, 0, MAX_SIZE_NAME_ELEMENT);

        printf("Выберите режим работы программы\n"
               "Для этого введите одну из цифр:\n"
               "1 - Угадывание объекта\n"
               "2 - Выдачи определения указанного объекта\n"
               "3 - Сравнения указанных объектов.\n");

        int symbol = getchar();
        flush_buffer();


        switch(symbol)
        {
            case '1':
            {
                printf("Выбран режим \"Угадывания объекта\"\n");

                storage_location_node = guess_answer(tree_pointer);

                printf("Вы загадали: %s?\n", (*storage_location_node)->data);

                if (check_answer_to_question()) {
                    break;
                }

                printf("Что вы загадали?\n");

                fgets_whithout_newline(name_of_element_1, MAX_SIZE_NAME_ELEMENT, stdin);

                printf("Что отличает %s от %s?\n", name_of_element_1, (*storage_location_node)->data);
                fgets_whithout_newline(distinctive_property, MAX_SIZE_NAME_ELEMENT, stdin);

                inserting_new_element(tree_pointer, storage_location_node, distinctive_property, name_of_element_1);

                break;
            }

            case '2':
            {
                printf("Выбран режим \"Выдачи определения\"\n"
                       "Введите имя объекта:\n");

                fgets_whithout_newline(name_of_element_1, MAX_SIZE_NAME_ELEMENT, stdin);

                define_element(tree_pointer, name_of_element_1);
                break;
            }

            case '3':
            {
                printf("Выбран режим \"Сравнения указанных объектов\"\n"
                       "Введите имя первого объекта:\n");
                fgets_whithout_newline(name_of_element_1, MAX_SIZE_NAME_ELEMENT, stdin);

                printf("Введите имя второго объекта:\n");
                fgets_whithout_newline(name_of_element_2, MAX_SIZE_NAME_ELEMENT, stdin);

                find_difference_between_elements(tree_pointer, name_of_element_1, name_of_element_2);

                break;
            }

            default:
                printf("Ошибка! Неправильно указан режим работы программы!\n");
        }

        printf("Хотите ли вы завершить программу?\n");

        if (check_answer_to_question()) {
            break;
        }

    } while(!user_wants_terminate_programm);

    printf("Хотите ли вы сохранить новую базу данных?\n");

    if (check_answer_to_question()) {
        print_node(tree_pointer->root, file_output);
    }

    tree_destructor(tree_pointer);

    MYASSERT(check_isclose(Global_logs_pointer_tree), COULD_NOT_CLOSE_THE_FILE , return COULD_NOT_CLOSE_THE_FILE);
    MYASSERT(check_isclose(file_input),               COULD_NOT_CLOSE_THE_FILE , return COULD_NOT_CLOSE_THE_FILE);
    MYASSERT(check_isclose(file_output),              COULD_NOT_CLOSE_THE_FILE , return COULD_NOT_CLOSE_THE_FILE);

    return 0;
}
