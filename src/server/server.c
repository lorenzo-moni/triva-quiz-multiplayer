#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include "utils/utils.h"
#include "signal.h"

#define PORT 8080

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

    // Creazione del socket del server
    if ((context.server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket fallita");
        exit(EXIT_FAILURE);
    }

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
    if (bind(context.server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
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

    // serve fare fare in modo che l'utente possa inserire una carattere alla volta sul terminale
    enable_raw_mode();

    // Ciclo principale del server
    while (1)
    {
        context.readfds = context.masterfds;

        show_dashboard(&context);

        activity = select(context.clientsInfo.max_fd + 1, &context.readfds, NULL, NULL, NULL);

        // controllo eventuali errori nella select
        if ((activity < 0) && (errno != EINTR))
        {
            perror("Select fallita");
            disable_raw_mode();
            exit(EXIT_FAILURE);
        }
        // controllo se l'utente ha digitato il carattere "q" per terminare il server
        if (FD_ISSET(STDIN_FILENO, &context.readfds))
        {
            char input;
            read(STDIN_FILENO, &input, 1);
            if (input == 'q')
                break;
        }

        // gestisco una nuova connessione sul server da parte di un utente
        if (FD_ISSET(context.server_fd, &context.readfds))
            handle_new_client_connection(&context);

        // ciclo all'interno della lista dei clients per gestire le richieste inviate sui rispettivi socket
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

    // dealloco i quiz
    deallocate_quizzes(&context.quizzesInfo);
    // dealloco i clients
    deallocate_clients(&context.clientsInfo);

    return 0;
}
