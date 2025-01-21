#include "common.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

void send_msg(int dest_fd, char *payload)
{
    size_t payload_length = strlen(payload);
    uint32_t net_payload_length = htonl(payload_length);

    if (send(dest_fd, &net_payload_length, sizeof(net_payload_length), 0) == -1)
    {
        perror("Errore durante l'invio della dimensione del pacchetto");
        return;
    }
    if (send(dest_fd, payload, payload_length, 0) == -1)
    {
        perror("Error durante l'invio del messaggio");
        return;
    }
}

void receive_msg(int source_fd, char *payload)
{
    uint32_t net_payload_length;

    ssize_t bytes_received = recv(source_fd, &net_payload_length, sizeof(net_payload_length), 0);

    if (bytes_received == 0)
        printf("L'altro lato ha chiuso la connessione");
    else if (bytes_received < 0)
        perror("C'è stato un errore durante la ricezione del messaggio");

    size_t payload_length = ntohl(net_payload_length);

    bytes_received = recv(source_fd, payload, payload_length, 0);

    if (bytes_received == 0)
        printf("L'altro lato ha chiuso la connessione");
    else if (bytes_received < 0)
        perror("C'è stato un errore durante la ricezione del messaggio");

    payload[payload_length] = '\0';
}