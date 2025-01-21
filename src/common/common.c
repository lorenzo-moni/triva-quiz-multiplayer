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

    // Controlla che il payload non superi la dimensione massima
    if (payload_len >= MAX_PAYLOAD_SIZE)
    {
        printf("Errore: Payload troppo grande\n");
        free(msg);
        return NULL;
    }

    // Copia il payload
    memcpy(msg->payload, payload, payload_len);
    msg->payload_length = payload_len;

    return msg;
}

void send_msg(int client_fd, Message *msg)
{
    uint32_t net_msg_type = htonl(msg->type);
    uint32_t net_msg_payload_length = htonl(msg->payload_length);

    if (send(client_fd, &net_msg_type, sizeof(net_msg_type), 0) == -1)
    {
        perror("Error in sending message type");
        free(msg);
        return;
    }
    if (send(client_fd, &net_msg_payload_length, sizeof(net_msg_payload_length),
             0) == -1)
    {
        perror("Error in sending message payload length");
        free(msg);
        return;
    }
    if (msg->payload_length > 0)
    {
        if (send(client_fd, msg->payload, msg->payload_length, 0) == -1)
        {
            perror("Error in sending payload");
            free(msg);
            return;
        }
    }
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

    bytes_received = recv(client_fd, &net_msg_payload_length,
                          sizeof(net_msg_payload_length), 0);

    if (bytes_received <= 0)
        return bytes_received;

    msg->payload_length = ntohl(net_msg_payload_length);

    if (msg->payload_length > MAX_PAYLOAD_SIZE)
    {
        fprintf(stderr, "Errore: Payload troppo grande (%d byte).\n",
                msg->payload_length);
        return -1;
    }

    if (msg->payload_length > 0)
    {
        bytes_received = recv(client_fd, msg->payload, msg->payload_length, 0);
        if (bytes_received <= 0)
            return bytes_received;

        msg->payload[msg->payload_length] = '\0';
    }
    else
    {
        msg->payload[0] = '\0'; // Nessun payload, stringa vuota
    }

    return 1;
}