#include "utils.h"
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "../../common/common.h"

void handle_nickname_selection(int server_fd, Message *msg)
{
    char nickname[MAX_PAYLOAD_SIZE];
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

void deserialize_quiz_list(Message *msg, char ***quizzes, int *total_quizzes)
{

    char *pointer = msg->payload;
    uint32_t net_strings_num, net_string_len, string_len;
    memcpy(&net_strings_num, pointer, sizeof(uint32_t));
    pointer += sizeof(uint32_t);
    *total_quizzes = ntohl(net_strings_num);

    *quizzes = malloc(*total_quizzes * sizeof(char *));

    for (int i = 0; i < *total_quizzes; i++)
    {
        memcpy(&net_string_len, pointer, sizeof(uint32_t));
        string_len = ntohl(net_string_len);
        pointer += sizeof(uint32_t);

        (*quizzes)[i] = malloc(string_len + 1);
        memcpy((*quizzes)[i], pointer, string_len);
        (*quizzes)[i][string_len] = '\0';
        pointer += string_len;
    }
}

void deserialize_rankings(Message *msg, char ***quizzes, int *total_quizzes)
{

    char *pointer = msg->payload;
    uint32_t net_strings_num, net_string_len, string_len;
    memcpy(&net_strings_num, pointer, sizeof(uint32_t));
    pointer += sizeof(uint32_t);
    *total_quizzes = ntohl(net_strings_num);

    *quizzes = malloc(*total_quizzes * sizeof(char *));

    for (int i = 0; i < *total_quizzes; i++)
    {
        memcpy(&net_string_len, pointer, sizeof(uint32_t));
        string_len = ntohl(net_string_len);
        pointer += sizeof(uint32_t);

        (*quizzes)[i] = malloc(string_len + 1);
        memcpy((*quizzes)[i], pointer, string_len);
        (*quizzes)[i][string_len] = '\0';
        pointer += string_len;
    }
}

void handle_rankings(Message *msg)
{

    char *pointer = msg->payload;
    uint32_t net_string_len, net_client_score, net_client_per_quiz_counter;
}

void handle_quiz_selection(int server_fd, Message *msg)
{
    char **quizzes_name;
    int total_quizzes;
    deserialize_quiz_list(msg, &quizzes_name, &total_quizzes);
    show_quiz_list(quizzes_name, total_quizzes);

    char answer[MAX_PAYLOAD_SIZE];
    printf("La tua scelta: ");
    get_console_input(answer, sizeof(answer));

    MessageType msg_type;

    if (strcmp(answer, ENDQUIZ) == 0)
        msg_type = MSG_DISCONNECT;
    else if (strcmp(answer, SHOWSCORE) == 0)
        msg_type = MSG_REQ_RANKING;
    else
        msg_type = MSG_QUIZ_SELECT;

    Message *reply_msg = create_msg(msg_type, answer, strlen(answer));
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
    char answer[MAX_PAYLOAD_SIZE];
    printf("\n%s", msg->payload);
    printf("\nRisposta: ");
    get_console_input(answer, sizeof(answer));

    MessageType msg_type;

    if (strcmp(answer, ENDQUIZ) == 0)
        msg_type = MSG_DISCONNECT;
    else if (strcmp(answer, SHOWSCORE) == 0)
        msg_type = MSG_REQ_RANKING;
    else
        msg_type = MSG_QUIZ_ANSWER;

    Message *answer_msg = create_msg(msg_type, answer, strlen(answer));
    send_msg(server_fd, answer_msg);
}
