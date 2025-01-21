#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "utils/utils.h"
#include "../common/common.h"

int main()
{
    int server_fd;
    struct sockaddr_in server_address;

    while (initial_menu())
    {

        // Creazione del socket TCP
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("Errore nella creazione del socket\n");
            exit(EXIT_FAILURE);
        }

        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(PORT);

        // Converte l'indirizzo IP
        if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0)
        {
            printf("Indirizzo non valido\n");
            exit(EXIT_FAILURE);
        }

        // Connessione al server
        if (connect(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        {
            printf("Connessione fallita\n");
            exit(EXIT_FAILURE);
        }

        handle_nickname_selection(server_fd);

        // while (handle_quiz_selection() && handle_quiz_game())
        //     ;

        close(server_fd);
        printf("\n");
    }
    return 0;
}