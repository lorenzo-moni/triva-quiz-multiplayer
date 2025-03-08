#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Enumeration that defines the types of messages exchanged between client and server
 *
 * This enumeration represents the different types of messages that can be
 * sent and received during communication between the client and the server in the quiz system.
 */
typedef enum MessageType
{
    MSG_REQ_NICKNAME,  /**< Message sent by the server to request the client's nickname */
    MSG_SET_NICKNAME,  /**< Message sent by the client to set the nickname */
    MSG_OK_NICKNAME,   /**< Message sent by the server to indicate that the chosen nickname is correct */
    MSG_REQ_QUIZ_LIST, /**< Message sent by the client to request the list of quizzes */
    MSG_RES_QUIZ_LIST, /**< Message sent by the server to forward the list of quizzes [BINARY PROTOCOL] */
    MSG_QUIZ_SELECT,   /**< Message sent by the client to select a quiz [BINARY PROTOCOL] */
    MSG_QUIZ_SELECTED, /**< Message sent by the server to the client to confirm the selected quiz */
    MSG_QUIZ_QUESTION, /**< Message sent by the server to forward the quiz question */
    MSG_QUIZ_ANSWER,   /**< Message sent by the client with the answer to the quiz question */
    MSG_REQ_RANKING,   /**< Message sent by the client to the server to request the ranking */
    MSG_RES_RANKING,   /**< Message sent by the server to the client with the ranking [BINARY PROTOCOL] */
    MSG_DISCONNECT,    /**< Message sent by the client to the server to indicate disconnection */
    MSG_INFO           /**< Message sent by the server with an informational message for the client */
} MessageType;

/**
 * @brief Structure representing a message exchanged between client and server
 *
 * In particular, the payload is allocated on the heap based on payload_length.
 */
typedef struct Message
{
    MessageType type;        /**< Type of the message */
    uint32_t payload_length; /**< Size of the payload in bytes */
    char *payload;           /**< Pointer to the message payload data */
} Message;

void handle_malloc_error(void *ptr, const char *error_string);
int receive_msg(int client_fd, Message *msg);
int send_msg(int client_fd, MessageType type, char *payload, size_t payload_len);
int get_console_input(char *buffer, int buffer_size);
void clear_input_buffer();

#endif
