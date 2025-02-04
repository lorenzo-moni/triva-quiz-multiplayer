#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "arpa/inet.h"
#include <stdlib.h>
#include "utils/utils.h"
#include "signal.h"
#include "../common/common.h"

int main(int argc, const char **argv)
{

    if (argc != 2)
    {
        printf("Uso: %s <porta>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_fd;
    struct sockaddr_in server_address;
    int choice;

    // ignoro il segnale SIGPIPE che viene inviato quando si tenta di scrivere
    // su un socket o una pipe che non ha più lettori attivi
    // in questo modo ignoro il segnale e non permetto al client di crashare nel caso il server chiuda la connessione
    signal(SIGPIPE, SIG_IGN);

    while (1)
    {
        Message received_msg;
        int ret;

        show_menu();
        scanf("%d", &choice);
        clear_input_buffer();
        printf("\n");

        if (choice == 2)
            break;
        else if (choice != 1)
        {
            printf("Opzione non corretta, riprova\n\n");
            continue;
        }

        // creo del socket TCP
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("Errore nella creazione del socket\n");
            exit(EXIT_FAILURE);
        }

        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(atoi(argv[1]));

        // converto l'indirizzo IP nella network version
        if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0)
        {
            printf("Indirizzo non valido\n");
            exit(EXIT_FAILURE);
        }

        // effettuo la connessione al server
        if (connect(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        {
            printf("Connessione fallita\n\n");
            continue;
        }

        while (1)
        {
            // ricevo il messaggio dal server e agisco di conseguenza
            ret = receive_msg(server_fd, &received_msg);
            if (ret == 0)
            {
                printf("\nIl server ha chiuso la connessione\n");
                free(received_msg.payload);
                break;
            }
            else if (ret == -1)
            {
                printf("Si è verificato un'errore nella ricezione del messaggio dal server\n");
                exit(EXIT_FAILURE);
            }

            switch (received_msg.type)
            {
            case MSG_REQ_NICKNAME:
                handle_nickname_selection(server_fd, &received_msg);
                break;
            case MSG_OK_NICKNAME:
                request_available_quizzes(server_fd);
                break;
            case MSG_RES_QUIZ_LIST:
                handle_quiz_selection(server_fd, &received_msg);
                break;
            case MSG_QUIZ_QUESTION:
                handle_quiz_question(server_fd, &received_msg);
                break;
            case MSG_INFO:
                handle_message(&received_msg);
                break;
            case MSG_QUIZ_SELECTED:
                handle_selected_quiz(&received_msg);
                break;
            case MSG_RES_RANKING:
                handle_rankings(&received_msg);
                break;

            default:
                break;
            }
            free(received_msg.payload);
        }
        close(server_fd);
        printf("\n");
    }
    return 0;
}