#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include "constants.h"
#include "utils/utils.h"

#define PORT 8080

int main()
{
    int server_fd, activity;
    ClientsInfo clientsInfo;
    QuizzesInfo quizzesInfo;
    init_clients_info(&clientsInfo, &quizzesInfo);
    struct sockaddr_in server_address;

    fd_set readfds, master; // Set di file descriptor per la `select()`

    load_quizzes_from_directory("./quizzes", &quizzesInfo);

    // for (int i = 0; i < quizzesInfo.total_quizzes; i++)
    // {
    //     Quiz *current_quiz = quizzesInfo.quizzes[i];
    //     printf("%s\n", current_quiz->name);
    //     for (int j = 0; j < current_quiz->total_questions; j++)
    //     {
    //         printf("%s : %s \n", current_quiz->questions[j],
    //                current_quiz->answers[j]);
    //     }
    //     printf("\n");
    // }

    FD_ZERO(&master);
    FD_ZERO(&readfds);

    // Creazione del socket del server
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket fallita");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("Errore nell'impostazione di SO_REUSEADDR");
        exit(EXIT_FAILURE);
    }

    // Configurazione del socket

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Binding del socket
    if (bind(server_fd, (struct sockaddr *)&server_address,
             sizeof(server_address)) < 0)
    {
        perror("Bind fallita");
        exit(EXIT_FAILURE);
    }

    // In ascolto
    if (listen(server_fd, 10) < 0)
    {
        perror("Listen fallita");
        exit(EXIT_FAILURE);
    }

    printf("DEBUG: Server in ascolto sulla porta %d...\n", PORT);

    FD_SET(server_fd, &master);
    clientsInfo.max_fd = server_fd;
    Client *clients;

    // Ciclo principale del server
    while (1)
    {
        readfds = master;

        show_dashboard(&quizzesInfo, &clientsInfo);

        // Aspetta eventi con `select()`
        activity = select(clientsInfo.max_fd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            perror("Select fallita");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(server_fd, &readfds))
            handle_new_client_connection(&clientsInfo, &master, server_fd);

        clients = clientsInfo.clients;
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i].socket_fd != -1 &&
                FD_ISSET(clients[i].socket_fd, &readfds))
                handle_client(&clients[i], &clientsInfo, &quizzesInfo, &master);
        }
    }

    return 0;
}
