#include "utils.h"

ClientNode *create_client_node(int socket_fd)
{
    ClientNode *new_node = (ClientNode *)malloc(sizeof(ClientNode));
    if (!new_node)
    {
        perror("Failed to create new ClientNode");
        exit(EXIT_FAILURE);
    }
    new_node->socket_fd = socket_fd;
    new_node->next_client = NULL;
    return new_node;
}

// Quiz
