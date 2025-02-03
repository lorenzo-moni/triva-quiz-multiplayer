#include "common.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

void handle_malloc_error(void *ptr, const char *error_string)
{
    if (!ptr)
    {
        printf("%s\n", error_string);
        exit(EXIT_FAILURE);
    }
}

Message *create_msg(MessageType type, char *payload, size_t payload_len)
{

    Message *msg = (Message *)malloc(sizeof(Message));
    handle_malloc_error(msg, "Errore nell'allocazione della memoria per il messaggio");

    msg->type = type;
    msg->payload_length = payload_len;
    msg->payload = (char *)malloc(payload_len);
    handle_malloc_error(msg->payload, "Errore nell'allocazione della memoria per il payload del messaggio");
    // Copia il payload
    memcpy(msg->payload, payload, payload_len);
    return msg;
}

void send_msg(int client_fd, Message *msg)
{
    uint32_t net_msg_type = htonl(msg->type);
    uint32_t net_msg_payload_length = htonl(msg->payload_length);

    if (send(client_fd, &net_msg_type, sizeof(net_msg_type), 0) == -1)
        goto cleanup;

    if (send(client_fd, &net_msg_payload_length, sizeof(net_msg_payload_length), 0) == -1)
        goto cleanup;

    if (msg->payload_length > 0)
        if (send(client_fd, msg->payload, msg->payload_length, 0) == -1)
            goto cleanup;

cleanup:
    free(msg->payload);
    free(msg);
}

int receive_msg(int client_fd, Message *msg)
{
    uint32_t net_msg_type;
    uint32_t net_msg_payload_length;

    ssize_t bytes_received = recv(client_fd, &net_msg_type, sizeof(net_msg_type), 0);
    if (bytes_received <= 0)
        return bytes_received;

    msg->type = ntohl(net_msg_type);

    bytes_received = recv(client_fd, &net_msg_payload_length, sizeof(net_msg_payload_length), 0);

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

    bytes_received = recv(client_fd, msg->payload, msg->payload_length, 0);
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