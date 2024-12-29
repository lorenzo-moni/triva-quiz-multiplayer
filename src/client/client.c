#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "utils/utils.h"

int main()
{
    int server_fd;
    struct sockaddr_in server_address;
    int choice;

    while (1)
    {
        show_menu();
        scanf("%d", &choice);
        printf("\n");

        if (choice == 2)
            break;
        else if (choice != 1)
        {
            printf("\nOpzione non corretta, riprova\n");
            continue;
        }

        // Creazione del socket TCP
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("Errore nella creazione del socket\n");
            return -1;
        }

        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(PORT);

        // Converte l'indirizzo IP
        if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0)
        {
            printf("Indirizzo non valido\n");
            return -1;
        }

        // Connessione al server
        if (connect(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        {
            printf("Connessione fallita\n");
            return -1;
        }

        while (1)
        {
            // ricevo il messaggio dal server e agisco di conseguenza
            Message *received_msg = (Message *)malloc(sizeof(Message));
            receive_msg(server_fd, received_msg);

            switch (received_msg->type)
            {
            case MSG_REQ_NICKNAME:
                handle_nickname_selection(server_fd, received_msg);
                break;
            case MSG_OK_NICKNAME:
                request_available_quizzes(server_fd);
                break;
            case MSG_RES_QUIZ_LIST:
                handle_quiz_selection(server_fd, received_msg);
                break;

            default:
                break;
            }

            free(received_msg);
        }
        close(server_fd);
    }
    return 0;
}