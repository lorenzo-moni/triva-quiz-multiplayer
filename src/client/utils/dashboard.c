#include <stdbool.h>
#include "utils.h"

/**
 * @brief Displays the client menu
 */
void show_menu()
{
    printf("Trivia Quiz\n");
    printf("+++++++++++++++++++++++++++\n");
    printf("Menu\n");
    printf("1 - Start a Trivia session\n");
    printf("2 - Exit\n");
    printf("+++++++++++++++++++++++++++\n");
    printf("Your choice: ");
}

/**
 * @brief Displays the selected quiz
 *
 * @param msg pointer to the message that contains the received quiz name
 */
void handle_selected_quiz(Message *msg)
{
    printf("\nQuiz - %s\n", msg->payload);
    printf("+++++++++++++++++++++++++++");
}
