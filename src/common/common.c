#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

/**
 * @brief Handles memory allocation errors
 *
 * This function is used when allocating a block of memory using malloc or realloc.
 * If the memory area is not allocated correctly, it terminates the program with an error message.
 *
 * @param ptr pointer to the allocated memory to be checked
 * @param error_string string containing the error message to be printed in case of failure
 */
void handle_malloc_error(void *ptr, const char *error_string)
{
    if (!ptr)
    {
        printf("%s\n", error_string);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Sends all required bytes on the socket
 *
 * This function is used in case send fails to transmit all the data on the socket,
 * and it allows sending data until all data has been transmitted.
 * It is particularly useful when the payload is large.
 *
 * @param dest_fd file descriptor to which the data should be sent
 * @param buffer buffer to be sent
 * @param length length of the buffer in bytes
 * @return the number of bytes sent, or -1 in case of error
 */
ssize_t send_all(int dest_fd, char *buffer, size_t length)
{
    size_t total_sent = 0;
    ssize_t bytes_sent;

    while (total_sent < length)
    {
        bytes_sent = send(dest_fd, buffer + total_sent, length - total_sent, 0);
        if (bytes_sent <= 0)
            return bytes_sent;
        total_sent += bytes_sent;
    }
    return total_sent;
}

/**
 * @brief Receives all required bytes on the socket
 *
 * This function is used in case recv fails to receive all the data on the socket,
 * and it allows receiving data until all data has been received.
 * It is particularly useful when the payload is large.
 *
 * @param source_fd file descriptor from which to receive the message
 * @param buffer buffer in which to store the data
 * @param length length of the buffer in bytes
 * @return the number of bytes received, or -1 in case of error
 */
ssize_t receive_all(int source_fd, char *buffer, size_t length)
{
    size_t total_received = 0;
    ssize_t bytes_received;

    while (total_received < length)
    {
        bytes_received = recv(source_fd, buffer + total_received, length - total_received, 0);
        if (bytes_received <= 0)
            return bytes_received;
        total_received += bytes_received;
    }
    return total_received;
}

/**
 * @brief Sends a message to the specified destination
 *
 * This function sends a message to the client identified by the provided file descriptor.
 * Numeric values are converted to network byte order before sending.
 *
 * @param dest_fd file descriptor to which the message should be sent
 * @param type type of the message to be sent
 * @param payload pointer to the payload data to be sent
 * @param payload_length length of the payload in bytes
 *
 * @return 1 if the message was sent successfully, or -1 in case of error
 */
int send_msg(int dest_fd, MessageType type, char *payload, size_t payload_length)
{
    uint8_t net_msg_type = type;
    uint32_t net_msg_payload_length = htonl(payload_length);

    if (send(dest_fd, &net_msg_type, sizeof(net_msg_type), 0) == -1)
        return -1;

    if (send(dest_fd, &net_msg_payload_length, sizeof(net_msg_payload_length), 0) == -1)
        return -1;

    if (payload_length > 0)
        if (send_all(dest_fd, payload, payload_length) == -1)
            return -1;
    return 1;
}

/**
 * @brief Receives a message from a specified source
 *
 * This function receives a message from the client identified by the provided file descriptor.
 * Numeric values are converted from network byte order to host byte order after reception.
 *
 * In particular, it handles whether to add a string terminator in the case where the message payload
 * is of text protocol type or binary protocol as in the case of MSG_RES_QUIZ_LIST and MSG_RES_RANKING.
 *
 * @param source_fd file descriptor from which to receive the message
 * @param msg pointer to the Message structure in which to store the received data
 *
 * @return 1 if the message was received successfully, 0 if the client closed the connection,
 *         or a negative value in case of error.
 */
int receive_msg(int source_fd, Message *msg)
{
    uint8_t net_msg_type;
    uint32_t net_msg_payload_length;

    ssize_t bytes_received = recv(source_fd, &net_msg_type, sizeof(net_msg_type), 0);
    if (bytes_received <= 0)
        return bytes_received;

    msg->type = net_msg_type;

    bytes_received = recv(source_fd, &net_msg_payload_length, sizeof(net_msg_payload_length), 0);

    if (bytes_received <= 0)
        return bytes_received;

    msg->payload_length = ntohl(net_msg_payload_length);
    msg->payload = NULL;

    // Add space for the string terminator only for messages that use the text protocol
    if (msg->type == MSG_RES_QUIZ_LIST || msg->type == MSG_RES_RANKING || msg->type == MSG_QUIZ_SELECT)
    {
        // If msg->payload_length is zero, a message with only the type has been sent, so no need to receive
        if (msg->payload_length == 0)
            return 1;
        msg->payload = (char *)malloc(msg->payload_length);
        handle_malloc_error(msg->payload, "Memory allocation error for the payload");
    }
    else
    {
        msg->payload = (char *)malloc(msg->payload_length + 1);
        handle_malloc_error(msg->payload, "Memory allocation error for the payload");
        // If msg->payload_length is zero, a message with only the type has been sent, so no need to receive
        if (msg->payload_length == 0)
        {
            msg->payload[0] = '\0';
            return 1;
        }
    }

    bytes_received = receive_all(source_fd, msg->payload, msg->payload_length);
    if (bytes_received <= 0)
    {
        free(msg->payload);
        return bytes_received;
    }
    // If the message uses the text protocol, add the string terminator to the payload
    if (!(msg->type == MSG_RES_QUIZ_LIST || msg->type == MSG_RES_RANKING || msg->type == MSG_QUIZ_SELECT))
        msg->payload[msg->payload_length] = '\0';

    return 1;
}

/**
 * @brief Clears the standard input buffer.
 *
 * This function reads and discards all characters present in the input buffer
 * until a newline character is encountered or the End Of File is reached.
 *
 * It is useful for removing any residual characters in the buffer after input operations,
 * preventing unwanted behavior in subsequent reads.
 *
 */
void clear_input_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

/**
 * @brief Reads a string from standard input with error handling
 *
 * This function reads a line of input from the console and stores it in the provided buffer.
 * If the length of the input exceeds the size of the buffer, the input buffer is cleared
 * and an error is returned. It also handles the case where the user enters no value.
 *
 * @param buffer pointer to the buffer in which to store the user's input
 * @param buffer_size size of the buffer in bytes
 * @return returns 1 if the input was read correctly, -1 in case of error
 *
 */
int get_console_input(char *buffer, int buffer_size)
{
    if (fgets(buffer, buffer_size, stdin))
    {
        // Find the position of the newline and replace it with '\0'
        size_t len = strcspn(buffer, "\n");
        if (buffer[len] == '\n')
            buffer[len] = '\0';
        else
        {
            clear_input_buffer();
            printf("Error: the input exceeds the maximum length of %d characters \n", buffer_size);
            return -1;
        }
        if (len == 0)
        {
            printf("Error: the input is empty\n");
            return -1;
        }
    }
    else
    {
        printf("Error reading input\n");
        return -1;
    }

    return 1;
}
