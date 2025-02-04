#include "utils.h"
#include "stdbool.h"

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
/**
 * @brief Pulisce il buffer di input standard.
 *
 * Questa funzione legge e scarta tutti i caratteri presenti nel buffer di input
 * fino al carattere di nuova linea o fino a raggiungere l'End Of File.
 *
 * È utile per rimuovere eventuali caratteri residui nel buffer dopo operazioni di input,
 * prevenendo comportamenti indesiderati in successive letture.
 *
 */
void clear_input_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

/**
 * @brief Legge una stringa dall'input standard con gestione degli errori
 *
 * Questa funzione legge una linea di input dalla console e la memorizza nel buffer fornito.
 * Se la lunghezza dell'input supera la dimensione del buffer, il buffer di input viene svuotato
 * e viene restituito un errore. Gestisce anche il caso in cui l'utente non inserisce alcun valore.
 *
 * @param buffer puntatore al buffer in cui memorizzare l'input dell'utente
 * @param buffer_size dimensione del buffer in byte
 * @return restituisce 1 se l'input è stato letto correttamente, -1 in caso di errore
 *
 */
int get_console_input(char *buffer, int buffer_size)
{
    if (fgets(buffer, buffer_size, stdin))
    {
        // trova la posizione del newline e la sostituisce con \0
        size_t len = strcspn(buffer, "\n");
        if (buffer[len] == '\n')
            buffer[len] = '\0';
        else
        {
            clear_input_buffer();
            printf("Errore: il valore inserito supera la lunghezza massima di %d caratteri \n", buffer_size);
            return -1;
        }
        if (len == 0)
        {
            printf("Errore: il valore inserito è vuoto\n");
            return -1;
        }
    }
    else
    {
        printf("Errore durante la lettura dell'input\n");
        return -1;
    }

    return 1;
}
