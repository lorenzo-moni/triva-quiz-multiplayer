#include "utils.h"
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "../../common/common.h"

void handle_nickname_selection(int server_fd)
{
    char nickname[MAX_PAYLOAD_SIZE], reply[MAX_PAYLOAD_SIZE];

    while (true)
    {
        printf("Inserisci il tuo nickname (max %d caratteri): ", MAX_PAYLOAD_SIZE - 1);
        get_console_input(nickname, sizeof(nickname), NULL);

        printf("nickanme %s\n", nickname);

        send_msg(server_fd, nickname);

        receive_msg(server_fd, reply);

        if (strcmp(reply, ERROR_CODE) == 0)
        {
            printf("\nNickname non valido\n\n");
            continue;
        }
        else if (strcmp(reply, SUCCESS_CODE) == 0)
        {
            printf("\nNickname valido\n\n");
            break;
        }
        else
        {
            printf("Errore durante la gestione del nickname da parte del server\n");
            exit(EXIT_FAILURE);
        }
    }
}
// void request_available_quizzes(int server_fd)
// {
//     Message *msg = create_msg(MSG_REQ_QUIZ_LIST, "", 0);
//     send_msg(server_fd, msg);
// }

// void deserialize_quiz_list(Message *msg, char ***quizzes, int *total_quizzes)
// {

//     char *pointer = msg->payload;
//     uint16_t net_strings_num, net_string_len, string_len;
//     memcpy(&net_strings_num, pointer, sizeof(uint16_t));
//     pointer += sizeof(uint16_t);
//     *total_quizzes = ntohs(net_strings_num);

//     *quizzes = malloc(*total_quizzes * sizeof(char *));

//     for (int i = 0; i < *total_quizzes; i++)
//     {
//         memcpy(&net_string_len, pointer, sizeof(uint16_t));
//         string_len = ntohs(net_string_len);
//         pointer += sizeof(uint16_t);

//         (*quizzes)[i] = malloc(string_len + 1);
//         memcpy((*quizzes)[i], pointer, string_len);
//         (*quizzes)[i][string_len] = '\0';
//         pointer += string_len;
//     }
// }

// void handle_quiz_selection(int server_fd, Message *msg, int *stop)
// {
//     char **quizzes_name;
//     int total_quizzes;
//     deserialize_quiz_list(msg, &quizzes_name, &total_quizzes);
//     show_quiz_list(quizzes_name, total_quizzes);

//     char answer[MAX_PAYLOAD_SIZE];
//     printf("La tua scelta: ");
//     get_console_input(answer, sizeof(answer), stop);

//     if (*stop)
//         return;

//     Message *reply_msg = create_msg(MSG_QUIZ_SELECT, answer, strlen(answer));
//     send_msg(server_fd, reply_msg);
// }

// void handle_error(Message *msg)
// {
//     printf("\n%s\n", msg->payload);
// }
// void handle_message(Message *msg)
// {
//     printf("\n%s\n", msg->payload);
// }

// void handle_quiz_question(int server_fd, Message *msg, int *stop)
// {
//     char answer[MAX_PAYLOAD_SIZE];
//     printf("\n%s", msg->payload);
//     printf("\nRisposta: ");
//     get_console_input(answer, sizeof(answer), stop);

//     if (*stop)
//         return;

//     Message *answer_msg = create_msg(MSG_QUIZ_ANSWER, answer, strlen(answer));
//     send_msg(server_fd, answer_msg);
// }
