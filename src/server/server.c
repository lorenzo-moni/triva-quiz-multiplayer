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
    int activity;
    Context context;
    struct sockaddr_in server_address;

    load_quizzes_from_directory("./quizzes", &context.quizzesInfo);
    init_clients_info(&context.clientsInfo);

    // for (int i = 0; i < quizzesInfo.total_quizzes; i++)
    // {
    //     Quiz *current_quiz = quizzesInfo.quizzes[i];
    //     printf("%s\n", current_quiz->name);
    //     for (int j = 0; j < current_quiz->total_questions; j++)
    //     {
    //         printf("Domanda: %s \n", current_quiz->questions[j]->question);
    //         for (int a = 0; a < current_quiz->questions[j]->total_answers; a++)
    //         {
    //             printf("Risposta: %s\n", current_quiz->questions[j]->answers[a]);
    //         }
    //     }
    //     printf("\n");
    // }

    FD_ZERO(&context.masterfds);
    FD_ZERO(&context.readfds);

    // Creazione del socket del server
    if ((context.server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket fallita");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(context.server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("Errore nell'impostazione di SO_REUSEADDR");
        exit(EXIT_FAILURE);
    }

    // Configurazione del socket

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Binding del socket
    if (bind(context.server_fd, (struct sockaddr *)&server_address,
             sizeof(server_address)) < 0)
    {
        perror("Bind fallita");
        exit(EXIT_FAILURE);
    }

    // In ascolto
    if (listen(context.server_fd, 10) < 0)
    {
        perror("Listen fallita");
        exit(EXIT_FAILURE);
    }

    printf("DEBUG: Server in ascolto sulla porta %d...\n", PORT);

    FD_SET(context.server_fd, &context.masterfds);
    FD_SET(STDIN_FILENO, &context.masterfds); // Aggiungi lo stdin per monitorare l'input
    context.clientsInfo.max_fd = context.server_fd > STDIN_FILENO ? context.server_fd : STDERR_FILENO;
    Client *client;

    enable_raw_mode();

    // Ciclo principale del server
    while (1)
    {
        context.readfds = context.masterfds;

        printf("context maxfd: %d\n", context.clientsInfo.max_fd);

        show_dashboard(&context);

        // Aspetta eventi con `select()`
        activity = select(context.clientsInfo.max_fd + 1, &context.readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            perror("Select fallita");
            disable_raw_mode();
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(STDIN_FILENO, &context.readfds))
        {
            char input;
            read(STDIN_FILENO, &input, 1);
            if (input == 'q')
                break;
        }

        if (FD_ISSET(context.server_fd, &context.readfds))
            handle_new_client_connection(&context);

        client = context.clientsInfo.clients_head;
        while (client)
        {
            if (FD_ISSET(client->socket_fd, &context.readfds))
                handle_client(client, &context);
            client = client->next_node;
        }
    }
    disable_raw_mode();
    printf("\nTerminazione del server\n");
    close(context.server_fd);

    deallocate_quizzes(&context.quizzesInfo);
    deallocate_clients(&context.clientsInfo);

    return 0;
}
