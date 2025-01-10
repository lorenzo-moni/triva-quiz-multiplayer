#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "../constants.h"
#include "utils.h"
#include <sys/select.h>

// La funzione init_clients_info va a gestire l'inizializzazione delle informazioni relative ai clienti che si possono connettere al sistema
// andando ad allocare dinamicamente le informazioni relative alla memorizzazione della posizione in classifica per ogni cliente in base al numero di quiz che sono
// stati caricati a runtime
void init_clients_info(ClientsInfo *clientsInfo, QuizzesInfo *quizzesInfo)
{
    clientsInfo->connected_clients = 0;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clientsInfo->clients[i].socket_fd = -1;
        clientsInfo->clients[i].client_rankings = malloc(quizzesInfo->total_quizzes * sizeof(RankingNode *));
        memset(clientsInfo->clients[i].client_rankings, 0, quizzesInfo->total_quizzes * sizeof(RankingNode *));
        clientsInfo->clients[i].current_quiz_id = -1;
    }
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

void send_msg(int client_fd, Message *msg)
{
    uint16_t net_msg_type = htons(msg->type);
    uint16_t net_msg_payload_length = htons(msg->payload_length);

    if (send(client_fd, &net_msg_type, sizeof(net_msg_type), 0) == -1)
    {
        perror("Error in sending message type");
        free(msg);
        return;
    }
    if (send(client_fd, &net_msg_payload_length, sizeof(net_msg_payload_length),
             0) == -1)
    {
        perror("Error in sending message payload length");
        free(msg);
        return;
    }
    if (msg->payload_length > 0)
    {
        if (send(client_fd, msg->payload, msg->payload_length, 0) == -1)
        {
            perror("Error in sending payload");
            free(msg);
            return;
        }
    }
    free(msg);
}

int receive_msg(int client_fd, Message *msg)
{
    uint16_t net_msg_type;
    uint16_t net_msg_payload_length;

    ssize_t bytes_received =
        recv(client_fd, &net_msg_type, sizeof(net_msg_type), 0);

    if (bytes_received <= 0)
    {
        if (bytes_received == 0)
            return 0;
        else if (bytes_received < 0)
            return -1;
    }

    msg->type = ntohs(net_msg_type);

    bytes_received = recv(client_fd, &net_msg_payload_length,
                          sizeof(net_msg_payload_length), 0);

    if (bytes_received <= 0)
    {
        if (bytes_received == 0)
            return 0;
        else if (bytes_received < 0)
            return -1;
    }

    msg->payload_length = ntohs(net_msg_payload_length);

    if (msg->payload_length > MAX_PAYLOAD_SIZE)
    {
        fprintf(stderr, "Errore: Payload troppo grande (%d byte).\n",
                msg->payload_length);
        return -1;
    }

    if (msg->payload_length > 0)
    {
        bytes_received = recv(client_fd, msg->payload, msg->payload_length, 0);
        if (bytes_received <= 0)
        {
            if (bytes_received <= 0)
            {
                if (bytes_received == 0)
                    return 0;
                else if (bytes_received < 0)
                    return -1;
            }
        }
        msg->payload[msg->payload_length] = '\0';
    }
    else
    {
        msg->payload[0] = '\0'; // Nessun payload, stringa vuota
    }

    return 1;
}

void request_client_nickname(int client_fd, int prev_invalid)
{
    char *message = prev_invalid ? "Il nickname selezionato non è disponibile, "
                                   "scegli un altro nickname"
                                 : "Scegli il tuo nickname (deve essere univoco)";
    Message *msg = create_msg(MSG_REQ_NICKNAME, message, strlen(message));
    send_msg(client_fd, msg);
}

void handle_new_client_connection(ClientsInfo *clientsInfo, fd_set *master, int server_fd)
{
    struct sockaddr_in client_address;
    int client_fd;
    socklen_t address_size = sizeof(client_address);
    client_fd = accept(server_fd, (struct sockaddr *)&client_address, &address_size);
    FD_SET(client_fd, master);
    if (client_fd > clientsInfo->max_fd)
        clientsInfo->max_fd = client_fd;

    Client *client = &clientsInfo->clients[client_fd];
    client->socket_fd = client_fd;
    client->state = WAITING_FOR_NICKNAME;

    request_client_nickname(client_fd, 0);
}

void send_quiz_list(Client *client, QuizzesInfo *quizzesInfo)
{
    // gestiamo la serializzazione della lista di quiz
    char *payload = malloc(MAX_PAYLOAD_SIZE);
    char *pointer = payload;
    size_t string_len;
    uint16_t net_string_len;

    uint16_t net_strings_num = htons(quizzesInfo->total_quizzes);
    memcpy(pointer, &net_strings_num, sizeof(uint16_t));
    pointer += sizeof(uint16_t);

    for (int i = 0; i < quizzesInfo->total_quizzes; i++)
    {
        printf("%s", quizzesInfo->quizzes[i]->name);
        string_len = strlen(quizzesInfo->quizzes[i]->name);
        net_string_len = htons(string_len);
        memcpy(pointer, &net_string_len, sizeof(uint16_t));
        pointer += sizeof(uint16_t);

        memcpy(pointer, quizzesInfo->quizzes[i]->name, string_len);
        pointer += string_len;
    }

    int payload_size = pointer - payload;

    Message *msg = create_msg(MSG_RES_QUIZ_LIST, payload, payload_size);
    send_msg(client->socket_fd, msg);
}

// questa funzione va a verificare se l'nickname è valido
void handle_client_nickname(Client *client, Message *received_msg,
                            ClientsInfo *clientsInfo)
{
    char *selected_nickname = received_msg->payload;
    int found = 0;
    for (int i = 0; i < clientsInfo->max_fd; i++)
    {
        if (clientsInfo->clients[i].socket_fd != -1 &&
            strcmp(selected_nickname, clientsInfo->clients[i].nickname) == 0)
        {
            found = 1;
            break;
        }
    }
    if (found)
    {
        request_client_nickname(client->socket_fd, 1);
        return;
    }
    clientsInfo->connected_clients += 1;
    client->nickname = malloc(strlen(selected_nickname) + 1);
    strcpy(client->nickname, selected_nickname);
    client->state = READY;

    Message *nickname_ok_msg = create_msg(MSG_OK_NICKNAME, "", 0);
    send_msg(client->socket_fd, nickname_ok_msg);
}

void handle_client_disconnection(Client *client, fd_set *master, ClientsInfo *clientsInfo, QuizzesInfo *quizzesInfo)
{
    FD_CLR(client->socket_fd, master);
    close(client->socket_fd);
    // aggiorno max_fd
    if (client->socket_fd == clientsInfo->max_fd)
    {
        clientsInfo->max_fd = -1;
        for (int i = 0; i < FD_SETSIZE; i++)
            if (FD_ISSET(i, master) && i > clientsInfo->max_fd)
                clientsInfo->max_fd = i;
    }
    // pulisco le strutture allocate in client
    client->current_quiz_id = -1;
    for (int i = 0; i < quizzesInfo->total_quizzes; i++)
    {
        remove_ranking(client->client_rankings[i], quizzesInfo->quizzes[i]);
        client->client_rankings[i] = NULL;
    }
    client->socket_fd = -1;
    if (client->nickname)
        free(client->nickname);
    clientsInfo->connected_clients--;
}

void send_quiz_question(Client *client, Quiz *quiz)
{
    int question_to_send_id = client->client_rankings[client->current_quiz_id]->current_question;
    if (question_to_send_id >= quiz->total_questions)
        return;
    char *question_to_send = quiz->questions[question_to_send_id];

    Message *msg = create_msg(MSG_QUIZ_QUESTION, question_to_send, strlen(question_to_send));
    send_msg(client->socket_fd, msg);
}

void verify_quiz_answer(Client *client, Message *msg, QuizzesInfo *quizzesInfo)
{
    char *user_answer = msg->payload;

    // if (strcmp(user_answer, "show score"))
    // {
    //     send_score();
    //     send_quiz_question();
    // }

    RankingNode *current_ranking = client->client_rankings[client->current_quiz_id];
    Quiz *playing_quiz = quizzesInfo->quizzes[client->current_quiz_id];
    char *right_answer = playing_quiz->answers[current_ranking->current_question];
    Message *result_msg;
    char *payload;

    if (strcmp(user_answer, right_answer) == 0)
    {
        payload = "Risposta corretta";
        current_ranking->score += 1;
        update_ranking(current_ranking, playing_quiz);
    }
    else
    {
        payload = "Risposta errata";
    }

    result_msg = create_msg(MSG_QUIZ_RESULT, payload, strlen(payload));
    send_msg(client->socket_fd, result_msg);

    current_ranking->current_question += 1;
    if (current_ranking->current_question == playing_quiz->total_questions)
    {
        client->state = READY;
        current_ranking->is_quiz_completed = 1;
        char *payload = "Hai terminato il quiz";
        Message *msg = create_msg(MSG_INFO, payload, strlen(payload));
        send_msg(client->socket_fd, msg);
        send_quiz_list(client, quizzesInfo);
    }

    else
        send_quiz_question(client, playing_quiz);
}

void handle_quiz_selection(Client *client, Message *msg, QuizzesInfo *quizzesInfo)
{
    if (strcmp(msg->payload, "show score") == 0)
    {
        return;
    }
    char *endptr;
    unsigned long value = strtoul(str, &endptr, 10);

    // Gestisco le possibili situazioni di errore

    // Il quiz indicato non è disponibile
    if (selected_quiz_number > quizzesInfo->total_quizzes)
    {
        char *message = "Quiz selezionato non valido";
        Message *error_msg = create_msg(MSG_ERROR, message, strlen(message));
        send_msg(client->socket_fd, error_msg);

        send_quiz_list(client, quizzesInfo);
        return;
    }

    // L'utente corrente ha già completato il quiz durante la sessione corrente
    if (client->client_rankings[selected_quiz_number - 1] != NULL)
    {
        char *message = "Il quiz selezionato è già stato completato in questa sessione";
        Message *error_msg = create_msg(MSG_ERROR, message, strlen(message));
        send_msg(client->socket_fd, error_msg);

        send_quiz_list(client, quizzesInfo);
        return;
    }
    client->state = PLAYING;
    client->current_quiz_id = selected_quiz_number - 1;

    // Inizializzo le informazioni relative al Ranking
    Quiz *selected_quiz = quizzesInfo->quizzes[selected_quiz_number - 1];
    RankingNode *new_node = create_ranking_node(client);
    client->client_rankings[client->current_quiz_id] = new_node;

    insert_ranking_node(selected_quiz, new_node);

    send_quiz_question(client, selected_quiz);
}

void handle_client(Client *client, ClientsInfo *clientsInfo, QuizzesInfo *quizzesInfo, fd_set *master)
{
    Message *received_msg = (Message *)malloc(sizeof(Message));
    int res = receive_msg(client->socket_fd, received_msg);
    if (res == 0)
    {
        printf("Il client ha chiuso la connessione\n");
        handle_client_disconnection(client, master, clientsInfo, quizzesInfo);
        free(received_msg);
        return;
    }
    else if (res == -1)
    {
        printf("Si è verificato un errore durante la ricezione di un messaggio\n");
        free(received_msg);
        return;
    }

    if (received_msg->type == MSG_SET_NICKNAME &&
        client->state == WAITING_FOR_NICKNAME)
        handle_client_nickname(client, received_msg, clientsInfo);
    else if (received_msg->type == MSG_REQ_QUIZ_LIST && client->state == READY)
        send_quiz_list(client, quizzesInfo);
    else if (received_msg->type == MSG_QUIZ_SELECT && client->state == READY)
        handle_quiz_selection(client, received_msg, quizzesInfo);

    else if (received_msg->type == MSG_QUIZ_ANSWER && client->state == PLAYING)
        verify_quiz_answer(client, received_msg, quizzesInfo);

    free(received_msg);
}