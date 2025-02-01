#include "utils.h"
#include "stdbool.h"

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

void handle_selected_quiz(Message *msg)
{
    printf("\nQuiz - %s\n", msg->payload);
    printf("+++++++++++++++++++++++++++");
}

void clear_input_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

void get_console_input(char *buffer, int buffer_size)
{
    if (fgets(buffer, buffer_size, stdin))
    {
        size_t len = strcspn(buffer, "\n"); // Trova la posizione di '\n'
        if (buffer[len] == '\n')
            buffer[len] = '\0'; // Sostituisci '\n' con il terminatore di stringa
        else
        {
            clear_input_buffer();
            printf("Errore: il valore inserito supera la lunghezza massima di %d caratteri \n", buffer_size);
            return;
        }
        if (len == 0)
        {
            printf("errore: il valore inserito è vuoto\n");
            return;
        }
    }
    else
    {
        printf("Errore durante la lettura dell'input\n");
    }

    return;
}
