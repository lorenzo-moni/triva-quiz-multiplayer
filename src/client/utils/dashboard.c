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

void clear_input_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

void get_console_input(char *buffer, int buffer_size, int *stop)
{
    if (fgets(buffer, buffer_size, stdin))
    {
        size_t len = strcspn(buffer, "\n"); // Trova la posizione di '\n'
        if (buffer[len] == '\n')
            buffer[len] = '\0'; // Sostituisci '\n' con il terminatore di stringa
        else
            clear_input_buffer();
    }
    else
    {
        // Errore nella lettura
        buffer[0] = '\0';
    }

    // Controlla la condizione di stop
    if (stop != NULL && strcmp(buffer, "endquiz") == 0)
    {
        *stop = 1;
        return;
    }
    return;
}