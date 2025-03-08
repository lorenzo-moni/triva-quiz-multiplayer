#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "arpa/inet.h"
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include "utils/utils.h"
#include "../common/common.h"
#include "../common/params.h"

int main(int argc, const char **argv)
{

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_fd;
    struct sockaddr_in server_address;
    int choice;

    // Ignore the SIGPIPE signal that is sent when attempting to write
    // to a socket or pipe that no longer has active readers.
    // This prevents the client from crashing if the server closes the connection.
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
            printf("Incorrect option, please try again\n\n");
            continue;
        }

        // Create a TCP socket
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("Error creating the socket\n");
            exit(EXIT_FAILURE);
        }

        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(atoi(argv[1]));

        // Convert the IP address to network format
        if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0)
        {
            printf("Invalid address\n");
            exit(EXIT_FAILURE);
        }

        // Establish the connection to the server
        if (connect(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        {
            printf("Connection failed\n\n");
            continue;
        }

        while (1)
        {
            // Receive the message from the server and act accordingly
            ret = receive_msg(server_fd, &received_msg);
            if (ret == 0)
            {
                printf("\nThe server has closed the connection\n");
                break;
            }

            else if (ret == -1)
            {
                if (errno == ECONNRESET || errno == ETIMEDOUT || errno == EPIPE)
                {
                    printf("The server closed the connection abnormally\n");
                    break;
                }
                else
                {
                    printf("Critical error\n");
                    exit(EXIT_FAILURE);
                }
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
