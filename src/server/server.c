#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include "utils/utils.h"
#include "../common/params.h"

int main()
{
    int activity;
    Context context;
    struct sockaddr_in server_address;
    int opt = 1;

    load_quizzes_from_directory("./quizzes", &context.quizzesInfo);
    init_clients_info(&context.clientsInfo);
    signal(SIGPIPE, SIG_IGN);

    FD_ZERO(&context.masterfds);
    FD_ZERO(&context.readfds);

    // Create the server socket
    if ((context.server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(context.server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("Error setting SO_REUSEADDR");
        exit(EXIT_FAILURE);
    }

    // Configure the socket
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr);
    server_address.sin_port = htons(SERVER_PORT);

    // Bind the socket
    if (bind(context.server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(context.server_fd, 10) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("DEBUG: Server listening on port %d...\n", SERVER_PORT);

    FD_SET(context.server_fd, &context.masterfds);
    FD_SET(STDIN_FILENO, &context.masterfds); // Add stdin to monitor input
    context.clientsInfo.max_fd = (context.server_fd > STDIN_FILENO) ? context.server_fd : STDIN_FILENO;
    Client *client, *next;

    // Main server loop
    while (1)
    {
        context.readfds = context.masterfds;

        show_dashboard(&context);

        activity = select(context.clientsInfo.max_fd + 1, &context.readfds, NULL, NULL, NULL);

        // Check for errors in select
        if ((activity < 0) && (errno != EINTR))
        {
            perror("Select failed");
            exit(EXIT_FAILURE);
        }
        // Check if the user typed the character "q" to terminate the server
        if (FD_ISSET(STDIN_FILENO, &context.readfds))
        {
            char buffer[DEFAULT_PAYLOAD_SIZE];
            if (get_console_input(buffer, sizeof(buffer)) == -1)
            {
                // If there is an error or EOF, remove STDIN from the master set
                FD_CLR(STDIN_FILENO, &context.masterfds);
            }
            else if (buffer[0] == 'q' && buffer[1] == '\0')
            {
                break;
            }
        }

        // Handle a new connection from a user on the server
        if (FD_ISSET(context.server_fd, &context.readfds))
            handle_new_client_connection(&context);

        // Loop through the client list to handle requests on their respective sockets
        client = context.clientsInfo.clients_head;
        while (client)
        {
            next = client->next_node;
            if (FD_ISSET(client->socket_fd, &context.readfds))
                handle_client(client, &context);
            client = next;
        }
    }

    printf("\nTerminating server\n");
    close(context.server_fd);

    // Deallocate the quizzes
    deallocate_quizzes(&context.quizzesInfo);
    // Deallocate the clients
    deallocate_clients(&context.clientsInfo);
    return 0;
}
