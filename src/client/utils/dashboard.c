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