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
 * @brief Invia un messaggio alla destinazione specificata
 *
 * Questa funzione invia un messaggio al client identificato dal file descriptor fornito.
 * I valori numerici vengono convertiti in network byte order prima dell'invio.
 *
 * @param client_fd file descriptor a cui inviare il messaggio
 * @param type tipo del messaggio da inviare
 * @param payload puntatore ai dati del payload da inviare
 * @param payload_length lunghezza del payload in byte
 */
void send_msg(int dest_fd, MessageType type, char *payload, size_t payload_length)
{
    uint32_t net_msg_type = htonl(type);
    uint32_t net_msg_payload_length = htonl(payload_length);

    if (send(dest_fd, &net_msg_type, sizeof(net_msg_type), 0) == -1)
        printf("Errore nell'invio del messaggio\n");

    if (send(dest_fd, &net_msg_payload_length, sizeof(net_msg_payload_length), 0) == -1)
        printf("Errore nell'invio del messaggio\n");

    if (payload_length > 0)
        if (send(dest_fd, payload, payload_length, 0) == -1)
            printf("Errore nell'invio del messaggio\n");
}
/**
 * @brief Riceve un messaggio da una sorgente specificata
 *
 * Questa funzione riceve un messaggio dal client identificato dal file descriptor fornito.
 * I valori numerici vengonoù convertiti da host byte order a network byte order dell'host dopo la ricezione.
 *
 * In particolare si gestisce l'aggiunta o meno del terminatore di stringa nel caso in cui il payload del messaggio
 * sia di tipo text protocol o binary protocol come nel caso di MSG_RES_QUIZ_LIST e MSG_RES_RANKING.
 *
 * @param source_fd file descriptor da cui ricevere il messaggio
 * @param msg Puntatore alla struttura Message in cui memorizzare i dati ricevuti
 *
 * @return Un valore positivo se la ricezione ha avuto successo, 0 se il client ha chiuso la connessione,
 *         o un valore negativo in caso di errore.
 */
int receive_msg(int source_fd, Message *msg)
{
    uint32_t net_msg_type;
    uint32_t net_msg_payload_length;

    ssize_t bytes_received = recv(source_fd, &net_msg_type, sizeof(net_msg_type), 0);
    if (bytes_received <= 0)
        return bytes_received;

    msg->type = ntohl(net_msg_type);

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

    bytes_received = recv(source_fd, msg->payload, msg->payload_length, 0);
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