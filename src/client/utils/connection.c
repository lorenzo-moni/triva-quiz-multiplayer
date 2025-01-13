#include "utils.h"
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

void sigpipe_handler(int sig)
{
    printf("Il server ha chiuso la connessione\n");
}

Message *create_msg(MessageType type, char *payload, size_t payload_len)
{

    Message *msg = (Message *)malloc(sizeof(Message));
    if (!msg)
    {
        perror("Errore nell'allocazione della memoria per il messaggio");
        return NULL;
    }

    msg->type = type;

    // Controlla che il payload non superi la dimensione massima
    if (payload_len >= MAX_PAYLOAD_SIZE)
    {
        fprintf(stderr, "Errore: Payload troppo grande.\n");
        free(msg);
        return NULL;
    }

    // Copia il payload
    memcpy(msg->payload, payload, payload_len);
    msg->payload_length = payload_len;

    return msg;
}

void send_msg(int server_fd, Message *msg)
{
    uint16_t net_msg_type = htons(msg->type);
    uint16_t net_msg_payload_length = htons(msg->payload_length);

    if (send(server_fd, &net_msg_type, sizeof(net_msg_type), 0) == -1)
    {
        perror("Error in sending message type");
        free(msg);
        return;
    }
    if (send(server_fd, &net_msg_payload_length, sizeof(net_msg_payload_length), 0) == -1)
    {
        perror("Error in sending message payload length");
        free(msg);
        return;
    }
    if (msg->payload_length > 0)
    {
        if (send(server_fd, msg->payload, msg->payload_length, 0) == -1)
        {
            perror("Error in sending payload");
            free(msg);
            return;
        }
    }
    free(msg);
}

int receive_msg(int server_fd, Message *msg)
{
    uint16_t net_msg_type;
    uint16_t net_msg_payload_length;

    ssize_t bytes_received = recv(server_fd, &net_msg_type, sizeof(net_msg_type), 0);

    if (bytes_received <= 0)
    {
        if (bytes_received == 0)
            printf("Il server ha chiuso la connessione");
        else if (bytes_received < 0)
            perror("C'è stato un errore durante la ricezione del messaggio");
        return -1;
    }

    msg->type = ntohs(net_msg_type);

    bytes_received = recv(server_fd, &net_msg_payload_length, sizeof(net_msg_payload_length), 0);

    if (bytes_received <= 0)
    {
        if (bytes_received == 0)
            printf("Il server ha chiuso la connessione");
        else if (bytes_received < 0)
            perror("C'è stato un errore durante la ricezione del messaggio");
        return -1;
    }

    msg->payload_length = ntohs(net_msg_payload_length);

    if (msg->payload_length > MAX_PAYLOAD_SIZE)
    {
        fprintf(stderr, "Errore: Payload troppo grande (%d byte).\n", msg->payload_length);
        return -1;
    }

    if (msg->payload_length > 0)
    {
        bytes_received = recv(server_fd, msg->payload, msg->payload_length, 0);
        if (bytes_received <= 0)
        {
            if (bytes_received == 0)
            {
                printf("Il server ha chiuso la connessione.\n");
            }
            else
            {
                perror("Errore durante la ricezione del payload");
            }
            return -1;
        }
        msg->payload[msg->payload_length] = '\0';
    }
    else
    {
        msg->payload[0] = '\0'; // Nessun payload, stringa vuota
    }

    return 0;
}

void handle_nickname_selection(int server_fd, Message *msg)
{
    char nickname[MAX_PAYLOAD_SIZE];
    printf("%s: ", msg->payload);

    get_console_input(nickname, sizeof(nickname), NULL);

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
    uint16_t net_strings_num, net_string_len, string_len;
    memcpy(&net_strings_num, pointer, sizeof(uint16_t));
    pointer += sizeof(uint16_t);
    *total_quizzes = ntohs(net_strings_num);

    *quizzes = malloc(*total_quizzes * sizeof(char *));

    for (int i = 0; i < *total_quizzes; i++)
    {
        memcpy(&net_string_len, pointer, sizeof(uint16_t));
        string_len = ntohs(net_string_len);
        pointer += sizeof(uint16_t);

        (*quizzes)[i] = malloc(string_len + 1);
        memcpy((*quizzes)[i], pointer, string_len);
        (*quizzes)[i][string_len] = '\0';
        pointer += string_len;
    }
}

void handle_quiz_selection(int server_fd, Message *msg, int *stop)
{
    char **quizzes_name;
    int total_quizzes;
    deserialize_quiz_list(msg, &quizzes_name, &total_quizzes);
    show_quiz_list(quizzes_name, total_quizzes);

    char answer[MAX_PAYLOAD_SIZE];
    printf("La tua scelta: ");
    get_console_input(answer, sizeof(answer), stop);

    if (*stop)
        return;

    Message *reply_msg = create_msg(MSG_QUIZ_SELECT, answer, strlen(answer));
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

void handle_quiz_question(int server_fd, Message *msg, int *stop)
{
    char answer[MAX_PAYLOAD_SIZE];
    printf("\n%s", msg->payload);
    printf("\nRisposta: ");
    get_console_input(answer, sizeof(answer), stop);

    if (*stop)
        return;

    Message *answer_msg = create_msg(MSG_QUIZ_ANSWER, answer, strlen(answer));
    send_msg(server_fd, answer_msg);
}
