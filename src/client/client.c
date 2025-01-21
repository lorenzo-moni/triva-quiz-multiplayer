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
    int choice;

    while (1)
    {
        show_menu();
        scanf("%d", &choice);
        clear_input_buffer();
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
        Message *received_msg = (Message *)malloc(sizeof(Message));
        int ret;

        while (1)
        {
            // ricevo il messaggio dal server e agisco di conseguenza
            ret = receive_msg(server_fd, received_msg);
            if (ret == 0)
            {
                printf("\nIl server ha chiuso la connessione\n");
                break;
            }
            else if (ret == -1)
            {
                printf("Si è verificato un'errore nella ricezione del messaggio dal server\n");
                exit(EXIT_FAILURE);
            }

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
            case MSG_QUIZ_QUESTION:
                handle_quiz_question(server_fd, received_msg);
                break;
            case MSG_QUIZ_RESULT:
            case MSG_INFO:
                handle_message(received_msg);
                break;
            case MSG_QUIZ_SELECTED:
                handle_selected_quiz(received_msg);
                break;
            case MSG_RES_RANKING:
                handle_rankings(received_msg);
                break;
            case MSG_ERROR:
                handle_error(received_msg);
                break;

            default:
                break;
            }
        }
        free(received_msg);
        close(server_fd);
        printf("\n");
    }
    return 0;
}