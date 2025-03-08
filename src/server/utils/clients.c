#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/select.h>
#include "../../common/common.h"
#include "../../common/params.h"
#include "utils.h"

/**
 * @brief Initializes the information related to the clients that can connect to the system
 *
 * @param clientsInfo pointer to the structure containing the clients' information
 */
void init_clients_info(ClientsInfo *clientsInfo)
{
    clientsInfo->connected_clients = 0;
    clientsInfo->clients_head = clientsInfo->clients_tail = NULL;
    clientsInfo->max_fd = 0;
}

/**
 * @brief Creates a new client node
 *
 * This function initializes a new client that has connected to the system.
 * In particular, it initializes the array containing all the RankingNode objects related to the client's games.
 *
 * @param client_fd file descriptor of the client's socket
 * @param quizzesInfo pointer to the structure containing the quiz information
 * @return Client structure representing the new connected client
 */
Client *create_client_node(int client_fd, QuizzesInfo *quizzesInfo)
{
    Client *new_client = (Client *)malloc(sizeof(Client));
    handle_malloc_error(new_client, "Memory allocation error for the new client");
    new_client->client_rankings = NULL;
    new_client->nickname = NULL;
    new_client->next_node = new_client->prev_node = NULL;
    new_client->current_quiz_id = -1;
    new_client->socket_fd = client_fd;
    new_client->state = LOGIN;
    new_client->client_rankings = malloc(quizzesInfo->total_quizzes * sizeof(RankingNode *));
    handle_malloc_error(new_client->client_rankings, "Memory allocation error for the new client's rankings");
    memset(new_client->client_rankings, 0, quizzesInfo->total_quizzes * sizeof(RankingNode *));
    return new_client;
}

/**
 * @brief Adds a Client structure to the list of connected clients
 *
 * This function inserts the client node at the tail of the doubly linked list of active clients in the system.
 *
 * @param node pointer to the Client structure to insert
 * @param clientsInfo pointer to the structure containing the clients' information
 */
void add_client(Client *node, ClientsInfo *clientsInfo)
{
    // Handle the case when the maximum number of clients is reached
    if (!node)
        return;

    if (!clientsInfo->clients_head)
    {
        clientsInfo->clients_head = clientsInfo->clients_tail = node;
        return;
    }
    clientsInfo->clients_tail->next_node = node;
    node->prev_node = clientsInfo->clients_tail;
    clientsInfo->clients_tail = node;
}

/**
 * @brief Removes and deallocates a Client structure from the list of connected clients
 *
 * @param node pointer to the Client structure to remove and deallocate
 * @param clientsInfo pointer to the structure containing the clients' information
 */
void remove_client(Client *node, ClientsInfo *clientsInfo)
{
    if (node == NULL)
        return;
    if (clientsInfo->clients_head == NULL)
        return;

    if (clientsInfo->clients_head == node)
        clientsInfo->clients_head = node->next_node;

    if (clientsInfo->clients_tail == node)
        clientsInfo->clients_tail = node->prev_node;

    if (node->prev_node != NULL)
        node->prev_node->next_node = node->next_node;

    if (node->next_node != NULL)
        node->next_node->prev_node = node->prev_node;

    free(node->nickname);
    free(node->client_rankings);
    free(node);
}

/**
 * @brief Removes and deallocates all clients from the list of clients connected to the system
 *
 * @param clientsInfo pointer to the structure containing the clients' information
 */
void deallocate_clients(ClientsInfo *clientsInfo)
{
    Client *current = clientsInfo->clients_head;
    Client *next;

    while (current != NULL)
    {
        next = current->next_node;
        free(current->client_rankings);
        free(current->nickname);
        free(current);
        current = next;
    }
}

/**
 * @brief Checks if the payload buffer has enough space and reallocates it if necessary
 *
 * This function is used to prevent buffer overflow in the functions send_quiz_list and send_ranking,
 * which use the binary protocol and serialization to send the quiz list and rankings respectively.
 *
 * The function checks whether the remaining space in the buffer, given by *pointer - *payload, is sufficient to hold the data to be copied,
 * with an additional extra_size.
 *
 * If the available space is insufficient, it doubles the buffer size until the required space is met, and then reallocates the payload buffer using realloc.
 *
 * @param payload pointer to a pointer to the start of the buffer
 * @param pointer pointer to a pointer to the current position in the buffer
 * @param buffer_size pointer to the current size of the buffer
 * @param extra_size additional space required
 */
void ensure_capacity(char **payload, char **pointer, size_t *buffer_size, size_t extra_size)
{
    // Calculate the used space and the required space
    size_t used_size = *pointer - *payload;
    size_t required_size = used_size + extra_size;

    // If there is already enough space, do nothing
    if (required_size <= *buffer_size)
        return;

    // Otherwise, double the buffer size until the required space is available
    size_t new_size = *buffer_size;
    while (new_size < required_size)
        new_size *= 2;

    // Reallocate the payload
    char *new_payload = (char *)realloc(*payload, new_size);
    handle_malloc_error(new_payload, "Error reallocating payload");

    // Restore the values for the calling function
    *payload = new_payload;
    *pointer = new_payload + used_size;
    *buffer_size = new_size;
}

/**
 * @brief Sends a message to the client to request the username
 *
 * This function sends a MSG_REQ_NICKNAME message to the client using the client_fd socket.
 *
 * @param client_fd file descriptor of the client from which the username is requested
 */
void request_client_nickname(int client_fd)
{
    char *message = "Choose a nickname (it must be unique): ";
    send_msg(client_fd, MSG_REQ_NICKNAME, message, strlen(message));
}

/**
 * @brief Handles the connection of a new client to the system
 *
 * This function is invoked when a new connection is detected on the server socket.
 * It accepts the connection and creates all the necessary data structures to manage the new client.
 *
 * @param context pointer to the structure that contains the service context information
 */
void handle_new_client_connection(Context *context)
{
    struct sockaddr_in client_address;
    int client_fd;
    socklen_t address_size = sizeof(client_address);
    // Accept the connection
    client_fd = accept(context->server_fd, (struct sockaddr *)&client_address, &address_size);
    if (client_fd == -1)
    {
        if (errno == EINTR)
            return;
        else
            exit(EXIT_FAILURE);
    }
    // Add the file descriptor to the master set
    FD_SET(client_fd, &context->masterfds);
    // Update max_fd
    if (client_fd > context->clientsInfo.max_fd)
        context->clientsInfo.max_fd = client_fd;

    // Create the client node and add it to the list
    Client *client = create_client_node(client_fd, &context->quizzesInfo);
    add_client(client, &context->clientsInfo);

    // Send the username request message to the client
    request_client_nickname(client_fd);
}

/**
 * @brief Sends the list of available quizzes to the client
 *
 * This function responds to a MSG_REQ_QUIZ_LIST message by serializing the list of available quizzes using the binary protocol
 * and sending them to the client.
 *
 * In particular, it uses the htons function to convert data from host byte order to network byte order,
 * and uses standardized uint16_t types to ensure portability.
 *
 * The quiz data will be serialized in the following binary format:
 * (number of quizzes) [(name length)(name)] [(name length)(name)] [...]
 * where the parentheses indicate the level of nesting and are not actually part of the transmitted data.
 *
 * @param client pointer to the client to which the list is sent
 * @param quizzesInfo pointer to the structure containing the quiz information
 */
void send_quiz_list(Client *client, QuizzesInfo *quizzesInfo)
{
    // Create a payload for our packet with a standard size
    size_t buffer_size = DEFAULT_PAYLOAD_SIZE;
    char *payload = (char *)malloc(buffer_size);
    handle_malloc_error(payload, "Error allocating payload");
    int payload_size;

    char *pointer = payload;
    size_t string_len;
    uint16_t net_string_len;
    Quiz *quiz;

    // Insert the total number of available quizzes into the buffer, ensuring there is enough space
    uint16_t net_strings_num = htons(quizzesInfo->total_quizzes);
    ensure_capacity(&payload, &pointer, &buffer_size, sizeof(uint16_t));
    memcpy(pointer, &net_strings_num, sizeof(uint16_t));
    pointer += sizeof(uint16_t);

    for (uint16_t i = 0; i < quizzesInfo->total_quizzes; i++)
    {
        quiz = quizzesInfo->quizzes[i];
        string_len = strlen(quiz->name);
        net_string_len = htons(string_len);

        // Handle the case where the previously allocated payload buffer is not large enough
        ensure_capacity(&payload, &pointer, &buffer_size, sizeof(uint16_t) + string_len);

        // Copy the string length into the buffer
        memcpy(pointer, &net_string_len, sizeof(uint16_t));
        pointer += sizeof(uint16_t);

        // Copy the string into the buffer
        memcpy(pointer, quiz->name, string_len);
        pointer += string_len;
    }

    payload_size = pointer - payload;

    client->state = SELECTING_QUIZ;

    // Send the serialized message
    send_msg(client->socket_fd, MSG_RES_QUIZ_LIST, payload, payload_size);
    free(payload);
}

/**
 * @brief Checks the validity of the nickname provided by the client
 *
 * This function is invoked after receiving a MSG_SET_NICKNAME message from the client
 * and checks whether the nickname is valid by verifying its uniqueness.
 *
 * @param client pointer to the client that sent the nickname
 * @param received_msg pointer to the received message
 * @param clientsInfo pointer to the structure containing the clients' information
 */
void handle_client_nickname(Client *client, Message *received_msg, ClientsInfo *clientsInfo)
{
    char *selected_nickname = received_msg->payload;
    bool found = 0;

    // Perform a linear search among clients that already have an associated nickname
    Client *current_client = clientsInfo->clients_head;
    while (current_client != NULL)
    {
        if (current_client->nickname && strcmp(selected_nickname, current_client->nickname) == 0)
        {
            found = true;
            break;
        }
        current_client = current_client->next_node;
    }
    if (found)
    {
        // If the nickname is already in use, send a message indicating the situation
        char *message = "Nickname already in use";
        send_msg(client->socket_fd, MSG_INFO, message, strlen(message));
        // Request a valid nickname from the client again
        request_client_nickname(client->socket_fd);
        return;
    }

    client->state = LOGGED_IN;
    clientsInfo->connected_clients += 1;
    client->nickname = malloc(strlen(selected_nickname) + 1);
    handle_malloc_error(client->nickname, "Error allocating memory for the client's nickname");
    strcpy(client->nickname, selected_nickname);

    // Send the correct nickname confirmation message to the client
    send_msg(client->socket_fd, MSG_OK_NICKNAME, "", 0);
}

/**
 * @brief Handles the disconnection of a client
 *
 * This function manages the disconnection of a client, which can occur either explicitly through a MSG_DISCONNECT message
 * or implicitly through direct socket closure.
 *
 * @param client pointer to the client to disconnect
 * @param context pointer to the structure containing the service context information
 */
void handle_client_disconnection(Client *client, Context *context)
{
    // Remove the client from the active file descriptor set and close the socket
    FD_CLR(client->socket_fd, &context->masterfds);
    close(client->socket_fd);

    // Remove all of the client's ranking entries
    for (uint16_t i = 0; i < context->quizzesInfo.total_quizzes; i++)
        remove_ranking(client->client_rankings[i], context->quizzesInfo.quizzes[i]);

    // Update max_fd
    if (client->socket_fd == context->clientsInfo.max_fd)
    {
        Client *current_client = context->clientsInfo.clients_head;
        context->clientsInfo.max_fd = context->server_fd > STDIN_FILENO ? context->server_fd : STDIN_FILENO;
        while (current_client)
        {
            if (current_client != client && current_client->socket_fd > context->clientsInfo.max_fd)
                context->clientsInfo.max_fd = current_client->socket_fd;

            current_client = current_client->next_node;
        }
    }

    if (client->state != LOGIN)
        context->clientsInfo.connected_clients--;
    remove_client(client, &context->clientsInfo);
}

/**
 * @brief Sends the current quiz question to the client
 *
 * This function sends a MSG_QUIZ_QUESTION message to the client containing the next question of the quiz they are participating in.
 *
 * @param client pointer to the client to whom the question is sent
 * @param quiz pointer to the structure containing the quiz information
 */
void send_quiz_question(Client *client, Quiz *quiz)
{
    uint16_t question_to_send_id = client->client_rankings[client->current_quiz_id]->current_question;
    if (question_to_send_id >= quiz->total_questions)
        return;
    // Select the correct question to send and send it to the client
    char *question_to_send = quiz->questions[question_to_send_id]->question;

    send_msg(client->socket_fd, MSG_QUIZ_QUESTION, question_to_send, strlen(question_to_send));
}

/**
 * @brief Verifies the client's answer
 *
 * This function checks if the answer is among the possible answers for that question.
 * Note that the answers are case insensitive.
 *
 * @param answer pointer to the client's answer
 * @param question pointer to the structure containing the quiz question
 */
bool verify_quiz_answer(char *answer, QuizQuestion *question)
{
    for (int a = 0; a < question->total_answers; a++)
        if (strcasecmp(answer, question->answers[a]) == 0)
            return true;
    return false;
}

/**
 * @brief Handles the client's answer message for a quiz question
 *
 * This function is invoked after receiving a MSG_QUIZ_ANSWER message from the client.
 * It checks the correctness of the answer, notifies the client of the result via a MSG_INFO message,
 * and updates the quiz ranking if the answer is correct.
 * Additionally, it sends the next question if the quiz is not finished; otherwise, it sends the quiz list again.
 *
 * @param client pointer to the client that sent the answer
 * @param msg pointer to the message containing the answer
 * @param quizzesInfo pointer to the structure containing the quiz information
 */
void handle_quiz_answer(Client *client, Message *msg, QuizzesInfo *quizzesInfo)
{
    char *user_answer = msg->payload;
    // Retrieve the RankingNode related to the quiz for which the client provided an answer
    RankingNode *current_ranking = client->client_rankings[client->current_quiz_id];
    // Retrieve the quiz the client is playing
    Quiz *playing_quiz = quizzesInfo->quizzes[client->current_quiz_id];
    // Retrieve the current question that the client answered
    QuizQuestion *current_question = playing_quiz->questions[current_ranking->current_question];
    char *payload;

    bool correct_answer = verify_quiz_answer(user_answer, current_question);
    // If the answer is correct, update the client's score and the ranking
    if (correct_answer)
    {
        payload = "Correct answer";
        current_ranking->score += 1;
        update_ranking(current_ranking, playing_quiz);
    }
    else
        payload = "Wrong answer";

    send_msg(client->socket_fd, MSG_INFO, payload, strlen(payload));

    current_ranking->current_question += 1;
    // If the client has finished the quiz, send the list of available quizzes; otherwise, send the next question
    if (current_ranking->current_question == playing_quiz->total_questions)
    {
        current_ranking->is_quiz_completed = true;
        payload = "You completed the quiz";
        send_msg(client->socket_fd, MSG_INFO, payload, strlen(payload));
        client->state = SELECTING_QUIZ;
        send_quiz_list(client, quizzesInfo);
    }
    else
        send_quiz_question(client, playing_quiz);
}

/**
 * @brief Handles the quiz selection by the client
 *
 * This function is invoked after receiving a MSG_QUIZ_SELECT message from the client,
 * and it initializes the necessary data structures for the quiz session.
 *
 * @param client pointer to the client who selected the quiz
 * @param msg pointer to the message containing the selected quiz
 * @param quizzesInfo pointer to the structure containing the quiz information
 */
void handle_quiz_selection(Client *client, Message *msg, QuizzesInfo *quizzesInfo)
{
    uint16_t net_selected_quiz_number, selected_quiz_number;
    memcpy(&net_selected_quiz_number, msg->payload, sizeof(net_selected_quiz_number));
    selected_quiz_number = ntohs(net_selected_quiz_number);

    // Handle possible error situations

    // The indicated quiz is not available
    if (selected_quiz_number > quizzesInfo->total_quizzes || selected_quiz_number == 0)
    {
        char *message = "Selected quiz is not valid";
        send_msg(client->socket_fd, MSG_INFO, message, strlen(message));
        send_quiz_list(client, quizzesInfo);
        return;
    }

    // The current user has already completed the quiz during this session
    if (client->client_rankings[selected_quiz_number - 1] != NULL)
    {
        char *message = "The selected quiz has already been completed in this session";
        send_msg(client->socket_fd, MSG_INFO, message, strlen(message));

        send_quiz_list(client, quizzesInfo);
        return;
    }
    client->current_quiz_id = selected_quiz_number - 1;

    // Initialize the ranking information
    Quiz *selected_quiz = quizzesInfo->quizzes[selected_quiz_number - 1];
    selected_quiz->total_clients += 1;
    RankingNode *new_node = create_ranking_node(client);
    client->client_rankings[client->current_quiz_id] = new_node;

    // Insert the node into the doubly linked ranking list for the quiz
    insert_ranking_node(selected_quiz, new_node);

    client->state = PLAYING;

    // Send the client a message confirming that a valid quiz has been selected
    send_msg(client->socket_fd, MSG_QUIZ_SELECTED, selected_quiz->name, strlen(selected_quiz->name));

    // Send the first question to the client
    send_quiz_question(client, selected_quiz);
}

/**
 * @brief Sends the ranking for each quiz to the client
 *
 * This function is invoked after receiving a MSG_REQ_RANKING message from the client,
 * and it serializes the clients' ranking for each quiz using the binary protocol and sends it to the client.
 *
 * In particular, it uses the htons function to convert data from host byte order to network byte order,
 * and uses standardized uint16_t types to ensure portability.
 *
 * The quiz ranking data will be serialized in the following binary protocol format:
 * (number of quizzes)  {(number of users participating in the quiz) [(name length) (name) (score)]} {...}
 * where the parentheses indicate the level of nesting and are not actually part of the transmitted data.
 *
 * @param client pointer to the client to which the ranking is sent
 * @param quizzesInfo pointer to the structure containing the quiz information
 */
void send_ranking(Client *client, QuizzesInfo *quizzesInfo)
{
    // Allocate a standard-sized buffer that can be expanded if needed
    size_t buffer_size = DEFAULT_PAYLOAD_SIZE;
    char *payload = (char *)malloc(buffer_size);
    handle_malloc_error(payload, "Error allocating payload");

    // Allocate the necessary data structures
    char *pointer = payload;
    size_t string_len;
    uint16_t net_string_len, net_client_score, net_clients_per_quiz;
    int payload_size;

    // Insert the total number of quizzes for which the ranking will be printed
    uint16_t net_quizzes_num = htons(quizzesInfo->total_quizzes);
    ensure_capacity(&payload, &pointer, &buffer_size, sizeof(uint16_t));
    memcpy(pointer, &net_quizzes_num, sizeof(uint16_t));
    pointer += sizeof(uint16_t);

    // For each quiz, insert in order: the number of users participating,
    // and for each user, the length of the nickname, the nickname, and the score
    for (uint16_t i = 0; i < quizzesInfo->total_quizzes; i++)
    {
        Quiz *quiz = quizzesInfo->quizzes[i];
        // Insert the number of clients in the ranking for the quiz
        net_clients_per_quiz = htons(quiz->total_clients);
        ensure_capacity(&payload, &pointer, &buffer_size, sizeof(uint16_t));
        memcpy(pointer, &net_clients_per_quiz, sizeof(uint16_t));
        pointer += sizeof(uint16_t);

        RankingNode *current = quiz->ranking_head;
        // Traverse the ranking list for the quiz
        while (current)
        {
            string_len = strlen(current->client->nickname);
            net_string_len = htons(string_len);

            // Handle the situation where the allocated buffer is not large enough
            ensure_capacity(&payload, &pointer, &buffer_size, sizeof(uint16_t) + string_len + sizeof(uint16_t));

            // Insert the length of the current client's nickname
            memcpy(pointer, &net_string_len, sizeof(uint16_t));
            pointer += sizeof(uint16_t);

            // Insert the current client's nickname
            memcpy(pointer, current->client->nickname, string_len);
            pointer += string_len;

            // Insert the current client's score
            net_client_score = htons(current->score);
            memcpy(pointer, &net_client_score, sizeof(uint16_t));
            pointer += sizeof(uint16_t);
            current = current->next_node;
        }
    }
    payload_size = pointer - payload;
    // Send the payload to the client in a MSG_RES_RANKING message
    send_msg(client->socket_fd, MSG_RES_RANKING, payload, payload_size);

    free(payload);

    // Based on the client's current state, send a different message
    switch (client->state)
    {
    case PLAYING:
        // If the client is in a quiz session, send the question to answer
        send_quiz_question(client, quizzesInfo->quizzes[client->current_quiz_id]);
        break;
    case SELECTING_QUIZ:
        // If the client is selecting a quiz, send the list of quizzes
        send_quiz_list(client, quizzesInfo);
        break;
    default:
        break;
    }
}

/**
 * @brief Handles the reception of a message from a client
 *
 * This function is invoked each time a socket is added to the readfds set, and it handles the client's request
 * based on the type of the received message.
 *
 * Using the receive_msg function, it obtains the message sent by the client and handles any disconnections or errors
 * that may occur during transmission.
 *
 * @param client pointer to the client that has been added to readfds
 * @param context pointer to the structure containing the service context information
 */
void handle_client(Client *client, Context *context)
{
    Message received_msg;
    int res = receive_msg(client->socket_fd, &received_msg);
    if (res == 0)
    {
        printf("The client closed the connection gracefully\n");
        handle_client_disconnection(client, context);
        return;
    }
    else if (res == -1)
    {
        if (errno == ECONNRESET || errno == ETIMEDOUT || errno == EPIPE)
        {
            printf("The client closed the connection abnormally\n");
            handle_client_disconnection(client, context);
            return;
        }
        else
        {
            printf("Critical error on the server\n");
            exit(EXIT_FAILURE);
        }
    }

    switch (received_msg.type)
    {
    case MSG_SET_NICKNAME:
        handle_client_nickname(client, &received_msg, &context->clientsInfo);
        break;
    case MSG_REQ_QUIZ_LIST:
        send_quiz_list(client, &context->quizzesInfo);
        break;
    case MSG_QUIZ_SELECT:
        handle_quiz_selection(client, &received_msg, &context->quizzesInfo);
        break;
    case MSG_QUIZ_ANSWER:
        handle_quiz_answer(client, &received_msg, &context->quizzesInfo);
        break;
    case MSG_REQ_RANKING:
        send_ranking(client, &context->quizzesInfo);
        break;
    case MSG_DISCONNECT:
        handle_client_disconnection(client, context);
        break;
    default:
        break;
    }
    free(received_msg.payload);
}
