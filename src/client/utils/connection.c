#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "utils.h"
#include "../../common/common.h"
#include "../../common/params.h"

/**
 * @brief Handles the user's nickname selection
 *
 * This function responds to the arrival of a MSG_REQ_NICKNAME message, through which the server requests the user to enter a nickname.
 * It handles cases where the input exceeds the buffer size or is empty using get_console_input, and sends the selected nickname
 * to the server via a MSG_SET_NICKNAME message.
 *
 * @param server_fd file descriptor of the server socket
 * @param msg pointer to the message containing the text to display
 */
void handle_nickname_selection(int server_fd, Message *msg)
{
    char nickname[DEFAULT_PAYLOAD_SIZE];
    do
        printf("%s", msg->payload);
    while (get_console_input(nickname, sizeof(nickname)) == -1);

    send_msg(server_fd, MSG_SET_NICKNAME, nickname, strlen(nickname));
}

/**
 * @brief Handles nickname confirmation and requests the available quiz list
 *
 * This function responds to the arrival of a MSG_OK_NICKNAME message, through which the server informs the user that the nickname has been accepted.
 * It responds by sending a request to the server for the list of available quizzes via a MSG_REQ_QUIZ_LIST message.
 *
 * @param server_fd file descriptor of the server socket
 */
void request_available_quizzes(int server_fd)
{
    send_msg(server_fd, MSG_REQ_QUIZ_LIST, "", 0);
}

/**
 * @brief Handles the deserialization and display of the list of available quizzes
 *
 * This function is responsible for deserializing, using a binary protocol, the list of available quizzes received from the server.
 *
 * In particular, it uses the ntohs function to convert from network byte order to host byte order
 * and utilizes standardized uint16_t types to ensure portability.
 *
 * The data regarding available quizzes is received according to this binary format:
 * (number of quizzes) [(name length)(name)] [(name length)(name)] [...]
 * where the parentheses indicate the level of nesting and are not actually part of the transmitted data.
 *
 * @param msg pointer to the message whose payload contains the serialized list of quizzes
 */
void display_quiz_list(Message *msg)
{
    // initialize data structures
    char *pointer = msg->payload;
    size_t string_len;
    uint16_t net_strings_num, net_string_len, total_quizzes;
    memcpy(&net_strings_num, pointer, sizeof(uint16_t));
    pointer += sizeof(uint16_t);
    // retrieve and convert from the buffer the total number of quizzes, then loop through them
    total_quizzes = ntohs(net_strings_num);

    printf("\nAvailable Quizzes\n");
    printf("+++++++++++++++++++++++++++\n");

    for (uint16_t i = 0; i < total_quizzes; i++)
    {
        memcpy(&net_string_len, pointer, sizeof(uint16_t));
        string_len = ntohs(net_string_len);
        pointer += sizeof(uint16_t);

        printf("%u - %.*s\n", (unsigned)(i + 1), (int)string_len, pointer);

        pointer += string_len;
    }

    printf("+++++++++++++++++++++++++++\n");
}

/**
 * @brief Handles the deserialization and display of the ranking
 *
 * This function responds to the arrival of a MSG_RES_RANKING message and is responsible for deserializing, using a binary protocol,
 * the ranking for each quiz received from the server.
 *
 * In particular, it uses the ntohs function to convert from network byte order to host byte order
 * and utilizes standardized uint16_t types to ensure portability.
 *
 * The ranking data for the quizzes is received according to this binary protocol format:
 * (number of quizzes)  {(number of users participating in the quiz) [(name length) (name) (score)]} {...}
 * where the parentheses indicate the level of nesting and are not actually part of the transmitted data.
 *
 * @param msg pointer to the message whose payload contains the serialized ranking
 */
void handle_rankings(Message *msg)
{
    char *pointer = msg->payload;
    uint16_t clients_per_quiz, quizzes_num, client_score, string_len;
    uint16_t net_string_len, net_client_score, net_clients_per_quiz, net_quizzes_num;
    // retrieve the total number of quizzes
    memcpy(&net_quizzes_num, pointer, sizeof(uint16_t));
    quizzes_num = ntohs(net_quizzes_num);
    pointer += sizeof(uint16_t);

    for (uint16_t i = 0; i < quizzes_num; i++)
    {
        memcpy(&net_clients_per_quiz, pointer, sizeof(uint16_t));
        clients_per_quiz = ntohs(net_clients_per_quiz);
        pointer += sizeof(uint16_t);
        // retrieve the number of clients participating in the quiz
        printf("\nTheme %d score\n", i + 1);
        for (uint16_t j = 0; j < clients_per_quiz; j++)
        {
            // retrieve the length of the client's nickname
            memcpy(&net_string_len, pointer, sizeof(uint16_t));
            string_len = ntohs(net_string_len);
            pointer += sizeof(uint16_t);

            // retrieve the client's score
            memcpy(&net_client_score, pointer + string_len, sizeof(uint16_t));
            client_score = ntohs(net_client_score);
            // print the nickname and score to the screen
            printf("- %.*s %d\n", string_len, pointer, client_score);
            pointer += string_len + sizeof(uint16_t);
        }
    }
}

/**
 * @brief Handles the client's quiz selection
 *
 * This function responds to the arrival of a MSG_RES_QUIZ_LIST message, through which the server communicates the serialized list of quizzes to the user,
 * whose deserialization and display is handled by the display_quiz_list function.
 *
 * The function allows the user to make a choice; specifically, the behavior changes based on the value entered by the client:
 *  - ENDQUIZ: sends a MSG_DISCONNECT message that causes the server to deallocate the client's structures and close the connection
 *  - SHOWSCORE: sends a MSG_REQ_RANKING message that causes the server to respond with a MSG_RES_RANKING message, thus sending the ranking
 *  - ELSE: sends a MSG_QUIZ_SELECT message with a payload based on the entered value, allowing the client to select the quiz to participate in using the binary protocol
 *
 * @param server_fd file descriptor of the server socket
 * @param msg pointer to the message whose payload contains the serialized list of quizzes
 */
void handle_quiz_selection(int server_fd, Message *msg)
{
    char answer[DEFAULT_PAYLOAD_SIZE];
    char *endptr;
    uint16_t selected_quiz_number, net_selected_quiz_number;
    while (1)
    {
        display_quiz_list(msg);

        do
            printf("Your choice: ");
        while (get_console_input(answer, sizeof(answer)) == -1);

        if (strcmp(answer, ENDQUIZ) == 0)
        {
            send_msg(server_fd, MSG_DISCONNECT, "", 0);
            break;
        }
        else if (strcmp(answer, SHOWSCORE) == 0)
        {
            send_msg(server_fd, MSG_REQ_RANKING, "", 0);
            break;
        }

        // strtoul tries to convert the received string to an unsigned long int in base 10
        // endptr is set to the last character that was not converted, so it can be used to
        // determine if the conversion was unsuccessful

        selected_quiz_number = (uint16_t)strtoul(answer, &endptr, 10);

        // If the value can be converted, send it to the server
        if (*endptr == '\0')
        {
            net_selected_quiz_number = htons(selected_quiz_number);
            send_msg(server_fd, MSG_QUIZ_SELECT, (char *)&net_selected_quiz_number, sizeof(net_selected_quiz_number));
            break;
        }
        // otherwise, print an error message due to the failed conversion
        printf("\nThe selected quiz is not valid\n");
    }
}

/**
 * @brief Handles the reception of an informational message
 *
 * This function responds to the arrival of a MSG_INFO message.
 * The client prints the message to the screen.
 *
 * @param msg pointer to the received message
 */
void handle_message(Message *msg)
{
    printf("\n%s\n", msg->payload);
}

/**
 * @brief Handles the client's response to a quiz question
 *
 * This function responds to the arrival of a MSG_QUIZ_QUESTION message, through which the server communicates the quiz question to the user.
 *
 * The function allows the user to make a choice; specifically, the behavior changes based on the value entered by the client:
 *  - ENDQUIZ: sends a MSG_DISCONNECT message that causes the server to deallocate the client's structures and close the connection
 *  - SHOWSCORE: sends a MSG_REQ_RANKING message that causes the server to respond with a MSG_RES_RANKING message, thus sending the ranking
 *  - ELSE: sends a MSG_QUIZ_ANSWER message with a payload based on the entered value, allowing the client to answer the question
 *
 * @param server_fd file descriptor of the server socket
 * @param msg pointer to the received message
 */
void handle_quiz_question(int server_fd, Message *msg)
{
    char answer[DEFAULT_PAYLOAD_SIZE];
    printf("\n%s\n", msg->payload);
    do
        printf("Answer: ");
    while (get_console_input(answer, sizeof(answer)) == -1);

    if (strcmp(answer, ENDQUIZ) == 0)
        send_msg(server_fd, MSG_DISCONNECT, "", 0);
    else if (strcmp(answer, SHOWSCORE) == 0)
        send_msg(server_fd, MSG_REQ_RANKING, "", 0);
    else
        send_msg(server_fd, MSG_QUIZ_ANSWER, answer, strlen(answer));
}
