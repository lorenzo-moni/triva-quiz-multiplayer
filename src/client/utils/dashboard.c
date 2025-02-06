#include <stdbool.h>
#include "utils.h"

/**
 * @brief Mostra il menu del client
 */
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

/**
 * @brief Mostra il quiz selezionato
 *
 * @param msg puntatore al messaggio che contiene il nome del quiz ricevuto
 */
void handle_selected_quiz(Message *msg)
{
    printf("\nQuiz - %s\n", msg->payload);
    printf("+++++++++++++++++++++++++++");
}
