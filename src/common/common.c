#include "common.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

/**
 * @brief Gestisce gli errori di allocazione della memoria
 *
 * Questa funzione viene usata quando si alloca una porzione di memoria tramite malloc o realloc.
 * Nel caso in cui l'area di memoria non venga allocata correttamente si termina il programma con un messaggio di errore.
 *
 * @param ptr puntatore alla memoria allocata da verificare
 * @param error_string stringa contenente il messaggio di errore da stampare in caso di fallimento
 */

void handle_malloc_error(void *ptr, const char *error_string)
{
    if (!ptr)
    {
        printf("%s\n", error_string);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Invia tutti i byte necessari sul socket
 *
 * Questa funzione viene utilizzata in caso la send non riesca ad inviare tutti i dati sul socket e permette di inviare dati
 * finché tutti i dati non sono stati inoltrati.
 * È particolarmente utile quando il payload è di grandi dimensioni.
 *
 * @param dest_fd file descriptor a cui inviare i dati
 * @param buffer buffer da inviare
 * @param length lunghezza del buffer in byte
 * @return il numero di byte inviati, oppure -1 in caso di errore
 */
ssize_t send_all(int dest_fd, char *buffer, size_t length)
{
    size_t total_sent = 0;
    ssize_t bytes_sent;

    while (total_sent < length)
    {
        bytes_sent = send(dest_fd, buffer + total_sent, length - total_sent, 0);
        if (bytes_sent <= 0)
            return bytes_sent;
        total_sent += bytes_sent;
    }
    return total_sent;
}

/**
 * @brief Riceve tutti i byte necessari sul socket
 *
 * Questa funzione viene utilizzata in caso la recv non riesca ad ricevere tutti i dati sul socket e permette di ricevere dati
 * finché tutti i dati non sono stati ricevuti.
 * È particolarmente utile quando il payload è di grandi dimensioni.
 *
 * @param source_fd file descriptor da cui ricevere il messaggio
 * @param buffer buffer da inviare
 * @param length lunghezza del buffer in byte
 * @return il numero di byte ricevuti, oppure -1 in caso di errore
 */
ssize_t receive_all(int source_fd, char *buffer, size_t length)
{
    size_t total_received = 0;
    ssize_t bytes_received;

    while (total_received < length)
    {
        bytes_received = recv(source_fd, buffer + total_received, length - total_received, 0);
        if (bytes_received <= 0)
            return bytes_received;
        total_received += bytes_received;
    }
    return total_received;
}
/**
 * @brief Invia un messaggio alla destinazione specificata
 *
 * Questa funzione invia un messaggio al client identificato dal file descriptor fornito.
 * I valori numerici vengono convertiti in network byte order prima dell'invio.
 *
 * @param dest_fd file descriptor a cui inviare il messaggio
 * @param type tipo del messaggio da inviare
 * @param payload puntatore ai dati del payload da inviare
 * @param payload_length lunghezza del payload in byte
 *
 * @return 1 nel caso l'invio abbia avuto successo e -1 nel caso di errore
 */
int send_msg(int dest_fd, MessageType type, char *payload, size_t payload_length)
{
    uint8_t net_msg_type = type;
    uint32_t net_msg_payload_length = htonl(payload_length);

    if (send(dest_fd, &net_msg_type, sizeof(net_msg_type), 0) == -1)
        return -1;

    if (send(dest_fd, &net_msg_payload_length, sizeof(net_msg_payload_length), 0) == -1)
        return -1;

    if (payload_length > 0)
        if (send_all(dest_fd, payload, payload_length) == -1)
            return -1;
    return 1;
}

/**
 * @brief Riceve un messaggio da una sorgente specificata
 *
 * Questa funzione riceve un messaggio dal client identificato dal file descriptor fornito.
 * I valori numerici vengono convertiti da network byte order a host byte order dopo la ricezione.
 *
 * In particolare si gestisce l'aggiunta o meno del terminatore di stringa nel caso in cui il payload del messaggio
 * sia di tipo text protocol o binary protocol come nel caso di MSG_RES_QUIZ_LIST e MSG_RES_RANKING.
 *
 * @param source_fd file descriptor da cui ricevere il messaggio
 * @param msg Puntatore alla struttura Message in cui memorizzare i dati ricevuti
 *
 * @return 1 se la ricezione ha avuto esito positivo, 0 se il client ha chiuso la connessione,
 *         o un valore negativo in caso di errore.
 */
int receive_msg(int source_fd, Message *msg)
{
    uint8_t net_msg_type;
    uint32_t net_msg_payload_length;

    ssize_t bytes_received = recv(source_fd, &net_msg_type, sizeof(net_msg_type), 0);
    if (bytes_received <= 0)
        return bytes_received;

    msg->type = net_msg_type;

    bytes_received = recv(source_fd, &net_msg_payload_length, sizeof(net_msg_payload_length), 0);

    if (bytes_received <= 0)
        return bytes_received;

    msg->payload_length = ntohl(net_msg_payload_length);
    msg->payload = NULL;

    // aggiungo lo spazio per il terminatore di stringa solamente nei messaggi che usano text protocol
    if (msg->type == MSG_RES_QUIZ_LIST || msg->type == MSG_RES_RANKING)
    {
        // se msg->payload_length è nulla si è inviato un messaggio con solo il tipo, non occorre fare receive
        if (msg->payload_length == 0)
            return 1;
        msg->payload = (char *)malloc(msg->payload_length);
        handle_malloc_error(msg->payload, "Errore nell'allocazione della memoria per il payload");
    }
    else
    {
        msg->payload = (char *)malloc(msg->payload_length + 1);
        handle_malloc_error(msg->payload, "Errore nell'allocazione della memoria per il payload");
        // se msg->payload_length è nulla si è inviato un messaggio con solo il tipo, non occorre fare receive
        if (msg->payload_length == 0)
        {
            msg->payload[0] = '\0';
            return 1;
        }
    }

    bytes_received = receive_all(source_fd, msg->payload, msg->payload_length);
    if (bytes_received <= 0)
    {
        free(msg->payload);
        return bytes_received;
    }
    // se il messaggio usa text protocol vado ad aggiungere il terminatore di stringa al payload
    if (!(msg->type == MSG_RES_QUIZ_LIST || msg->type == MSG_RES_RANKING))
        msg->payload[msg->payload_length] = '\0';

    return 1;
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
