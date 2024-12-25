#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <errno.h>
#include "utils/utils.h"

#define PORT 8080
#define MAX_CLIENTS 1024
#define BUFFER_SIZE 1024

int main()
{
    int server_fd, new_socket, max_sd, activity, sd;
    int client_sockets[MAX_CLIENTS] = {0}; // Array per gestire i socket dei client
    struct sockaddr_in address;
    fd_set readfds; // Set di file descriptor per la `select()`
    char buffer[BUFFER_SIZE];

    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        printf("Directory di lavoro corrente: %s\n", cwd);
    }
    else
    {
        perror("Errore nel recupero della directory corrente");
    }

    int total_quizzes;
    printf("1\n");
    Quiz **quiz_array = load_quizzes_from_directory("./quizzes", &total_quizzes);

    for (int i = 0; i < total_quizzes; i++)
    {
        Quiz *current_quiz = quiz_array[i];
        printf("%s\n", current_quiz->name);
        for (int j = 0; j < current_quiz->total_questions; j++)
        {
            printf("%s : %s \n", current_quiz->questions[j], current_quiz->answers[j]);
        }
        printf("\n");
    }

    // Creazione del socket del server
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket fallita");
        exit(EXIT_FAILURE);
    }

    // Configurazione del socket
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("SetSockOpt fallita");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Binding del socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
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

    printf("Server in ascolto sulla porta %d...\n", PORT);

    // Ciclo principale del server
    while (1)
    {
        // Inizializza il set di file descriptor
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds); // Aggiungi il socket del server
        max_sd = server_fd;

        // Aggiungi i socket dei client al set
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            sd = client_sockets[i];
            if (sd > 0)
                FD_SET(sd, &readfds);
            if (sd > max_sd)
                max_sd = sd;
        }

        // Aspetta eventi con `select()`
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            perror("Select fallita");
            exit(EXIT_FAILURE);
        }

        // Controlla nuove connessioni
        if (FD_ISSET(server_fd, &readfds))
        {
            int addrlen = sizeof(address);
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
            {
                perror("Accept fallita");
                exit(EXIT_FAILURE);
            }

            printf("Nuovo client connesso: socket %d, IP %s, PORT %d\n",
                   new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // Aggiungi il nuovo socket all'array dei client
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_sockets[i] == 0)
                {
                    client_sockets[i] = new_socket;
                    printf("Aggiunto a client_sockets[%d]\n", i);
                    break;
                }
            }
        }

        // Gestisci I/O sui socket esistenti
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            sd = client_sockets[i];

            if (FD_ISSET(sd, &readfds))
            {
                int valread = read(sd, buffer, BUFFER_SIZE);
                if (valread == 0)
                {
                    // Client disconnesso
                    int addrlen = sizeof(address);
                    getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                    printf("Client disconnesso: IP %s, PORT %d\n",
                           inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    close(sd);
                    client_sockets[i] = 0;
                }
                else
                {
                    // Messaggio ricevuto
                    buffer[valread] = '\0';
                    printf("Messaggio ricevuto da socket %d: %s\n", sd, buffer);

                    // Rispondi al client
                    char response[] = "Risposta dal server\n";
                    send(sd, response, strlen(response), 0);
                }
            }
        }
    }

    return 0;
}
