#include "utils.h"
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "../../common/common.h"

void handle_nickname_selection(int server_fd, Message *msg)
{
    char nickname[DEFAULT_PAYLOAD_SIZE];
    printf("%s", msg->payload);

    get_console_input(nickname, sizeof(nickname));

    Message *reply_msg = create_msg(MSG_SET_NICKNAME, nickname, strlen(nickname));
    send_msg(server_fd, reply_msg);
}

void request_available_quizzes(int server_fd)
{
    Message *msg = create_msg(MSG_REQ_QUIZ_LIST, "", 0);
    send_msg(server_fd, msg);
}

void display_quiz_list(Message *msg)
{

    char *pointer = msg->payload;
    int string_len, total_quizzes;
    uint32_t net_strings_num, net_string_len;
    memcpy(&net_strings_num, pointer, sizeof(uint32_t));
    pointer += sizeof(uint32_t);
    total_quizzes = ntohl(net_strings_num);

    printf("\nQuiz disponibili\n");
    printf("+++++++++++++++++++++++++++\n");

    for (int i = 0; i < total_quizzes; i++)
    {
        memcpy(&net_string_len, pointer, sizeof(uint32_t));
        string_len = ntohl(net_string_len);
        pointer += sizeof(uint32_t);

        printf("%d - %.*s\n", i + 1, string_len, pointer);

        pointer += string_len;
    }

    printf("+++++++++++++++++++++++++++\n");
}

void handle_rankings(Message *msg)
{

    char *pointer = msg->payload;
    int clients_per_quiz, quizzes_num, client_score, string_len;
    uint32_t net_string_len, net_client_score, net_clients_per_quiz, net_quizzes_num;

    memcpy(&net_quizzes_num, pointer, sizeof(uint32_t));
    quizzes_num = ntohl(net_quizzes_num);
    pointer += sizeof(uint32_t);

    for (int i = 0; i < quizzes_num; i++)
    {
        memcpy(&net_clients_per_quiz, pointer, sizeof(uint32_t));
        clients_per_quiz = ntohl(net_clients_per_quiz);
        pointer += sizeof(uint32_t);
        printf("\nPunteggio Tema %d\n", i + 1);
        for (int j = 0; j < clients_per_quiz; j++)
        {
            memcpy(&net_string_len, pointer, sizeof(uint32_t));
            string_len = ntohl(net_string_len);
            pointer += sizeof(uint32_t);

            memcpy(&net_client_score, pointer + string_len, sizeof(uint32_t));
            client_score = ntohl(net_client_score);
            printf("- %.*s %d\n", string_len, pointer, client_score);
            pointer += string_len + sizeof(uint32_t);
        }
    }
}

void handle_quiz_selection(int server_fd, Message *msg)
{

    display_quiz_list(msg);

    char answer[DEFAULT_PAYLOAD_SIZE];
    printf("La tua scelta: ");
    get_console_input(answer, sizeof(answer));

    Message *reply_msg;

    if (strcmp(answer, ENDQUIZ) == 0)
        reply_msg = create_msg(MSG_DISCONNECT, "", 0);
    else if (strcmp(answer, SHOWSCORE) == 0)
        reply_msg = create_msg(MSG_REQ_RANKING, "", 0);
    else
        reply_msg = create_msg(MSG_QUIZ_SELECT, answer, strlen(answer));

    send_msg(server_fd, reply_msg);
}

void handle_error(Message *msg)
{
    printf("\n%s\n", msg->payload);
}
void handle_message(Message *msg)
{
    printf("\n%s\n", msg->payload);
}

void handle_quiz_question(int server_fd, Message *msg)
{
    char answer[DEFAULT_PAYLOAD_SIZE];
    printf("\n%s", msg->payload);
    printf("\nRisposta: ");
    get_console_input(answer, sizeof(answer));

    Message *reply_msg;

    if (strcmp(answer, ENDQUIZ) == 0)
        reply_msg = create_msg(MSG_DISCONNECT, "", 0);
    else if (strcmp(answer, SHOWSCORE) == 0)
        reply_msg = create_msg(MSG_REQ_RANKING, "", 0);
    else
        reply_msg = create_msg(MSG_QUIZ_ANSWER, answer, strlen(answer));

    send_msg(server_fd, reply_msg);
}
