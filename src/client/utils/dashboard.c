#include "utils.h"

void show_menu()
{
    printf("Trivia Quiz\n");
    printf("+++++++++++++++++++++++++++\n");
    printf("Menù\n");
    printf("1 - Comincia una sessione Trivia\n");
    printf("2 - Esci\n");
    printf("+++++++++++++++++++++++++++\n");
    printf("La tua scelta: ");
}

void show_quiz_list(char **quizzes, int total_quizzes)
{
    printf("\nQuiz disponibili\n");
    printf("+++++++++++++++++++++++++++\n");
    for (int i = 0; i < total_quizzes; i++)
    {
        printf("%d - %s\n", i + 1, quizzes[i]);
    }
    printf("+++++++++++++++++++++++++++\n");
}

void get_console_input(char *buffer, int buffer_size, int *stop, int can_request_score)
{

    fgets(input, buffer_size, stdin);
    size_t len = strcspn(input, "\n"); // Trova la posizione di '\n'
    if (input[len] == '\n')
        input[len] = '\0'; // Sostituisci '\n' con il terminatore di stringa

    if (strcmp(input, "endquiz") == 0)
    {
        *stop = 1;
        return;
    }

    //     if (can_request_score && strcmp(input, "show scores") == 0)
    //     {
    //     }
    return;
}