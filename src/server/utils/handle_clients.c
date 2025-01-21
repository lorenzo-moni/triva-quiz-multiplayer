#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "../../common/common.h"
#include "../constants.h"
#include "utils.h"
#include <sys/select.h>

// La funzione init_clients_info va a gestire l'inizializzazione delle informazioni relative ai clienti che si possono connettere al sistema
// andando ad allocare dinamicamente le informazioni relative alla memorizzazione della posizione in classifica per ogni cliente in base al numero di quiz che sono
// stati caricati a runtime
void init_clients_info(ClientsInfo *clientsInfo)
{
    clientsInfo->connected_clients = 0;
    clientsInfo->clients_head = clientsInfo->clients_tail = NULL;
    clientsInfo->max_fd = 0;
}

Client *create_client_node(int client_fd)
{
    Client *new_client = (Client *)malloc(sizeof(Client));
    new_client->client_rankings = NULL;
    new_client->nickname = NULL;
    new_client->next_node = new_client->prev_node = NULL;
    new_client->current_quiz_id = -1;
    new_client->socket_fd = client_fd;
    new_client->state = LOGIN;
    return new_client;
}

void add_client(Client *node, ClientsInfo *clientsInfo)
{
    // devo gestire il caso in cui si abbia il numero massimo di clients
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

void remove_client(Client *node, ClientsInfo *clientsInfo)
{
    if (node == NULL)
        return;
    if (clientsInfo->clients_head == NULL)
        return;

    if (clientsInfo->clients_head == node)
        clientsInfo->clients_head = node->next_node;

    if (node->prev_node != NULL)
        node->prev_node->next_node = node->next_node;

    if (node->next_node != NULL)
        node->next_node->prev_node = node->prev_node;

    free(node->nickname);
    free(node);
}

void handle_new_client_connection(Context *context, int server_fd)
{
    struct sockaddr_in client_address;
    int client_fd;
    socklen_t address_size = sizeof(client_address);
    client_fd = accept(server_fd, (struct sockaddr *)&client_address, &address_size);
    FD_SET(client_fd, &context->masterfds);
    if (client_fd > context->clientsInfo.max_fd)
        context->clientsInfo.max_fd = client_fd;

    Client *client = create_client_node(client_fd);
    printf("Nuovo client creato con fd %d rispetto a %d\n", client->socket_fd, client_fd);
    add_client(client, &context->clientsInfo);
}

// void send_quiz_list(Client *client, QuizzesInfo *quizzesInfo)
// {
//     // gestiamo la serializzazione della lista di quiz
//     char *payload = malloc(MAX_PAYLOAD_SIZE);
//     char *pointer = payload;
//     size_t string_len;
//     uint16_t net_string_len;

//     uint16_t net_strings_num = htons(quizzesInfo->total_quizzes);
//     memcpy(pointer, &net_strings_num, sizeof(uint16_t));
//     pointer += sizeof(uint16_t);

//     for (int i = 0; i < quizzesInfo->total_quizzes; i++)
//     {
//         string_len = strlen(quizzesInfo->quizzes[i]->name);
//         net_string_len = htons(string_len);
//         memcpy(pointer, &net_string_len, sizeof(uint16_t));
//         pointer += sizeof(uint16_t);

//         memcpy(pointer, quizzesInfo->quizzes[i]->name, string_len);
//         pointer += string_len;
//     }

//     int payload_size = pointer - payload;

//     client->state = SELECTING_QUIZ;

//     Message *msg = create_msg(MSG_RES_QUIZ_LIST, payload, payload_size);
//     send_msg(client->socket_fd, msg);
// }

// // questa funzione va a verificare se l'nickname è valido
void handle_client_login(Client *client, ClientsInfo *clientsInfo)
{
    char selected_nickname[MAX_PAYLOAD_SIZE];

    receive_msg(client->socket_fd, selected_nickname);

    int found = 0;

    Client *current_client = clientsInfo->clients_head;
    while (current_client != NULL)
    {
        if (current_client->state != LOGIN && strcmp(selected_nickname, current_client->nickname) == 0)
        {
            found = 1;
            break;
        }
        current_client = current_client->next_node;
    }
    if (found)
    {
        send_msg(client->socket_fd, ERROR_CODE);
        return;
    }
    client->state = LOGGED_IN;
    clientsInfo->connected_clients += 1;
    client->nickname = malloc(strlen(selected_nickname) + 1);
    strcpy(client->nickname, selected_nickname);

    send_msg(client->socket_fd, SUCCESS_CODE);
}

void handle_client_disconnection(Client *client, fd_set *master, ClientsInfo *clientsInfo, QuizzesInfo *quizzesInfo)
{
    FD_CLR(client->socket_fd, master);
    close(client->socket_fd);
    // aggiorno max_fd
    if (client->socket_fd == clientsInfo->max_fd)
    {
        Client *current_client = clientsInfo->clients_head;
        clientsInfo->max_fd = STDIN_FILENO;
        while (current_client)
        {
            if (current_client != client && current_client->socket_fd > clientsInfo->max_fd)
                clientsInfo->max_fd = current_client->socket_fd;
            current_client = current_client->next_node;
        }
    }
    // pulisco le strutture allocate in client
    for (int i = 0; i < quizzesInfo->total_quizzes; i++)
    {
        remove_ranking(client->client_rankings[i], quizzesInfo->quizzes[i]);
    }

    remove_client(client, clientsInfo);
    clientsInfo->connected_clients--;
}

// void send_quiz_question(Client *client, Quiz *quiz)
// {
//     int question_to_send_id = client->client_rankings[client->current_quiz_id]->current_question;
//     if (question_to_send_id >= quiz->total_questions)
//         return;
//     char *question_to_send = quiz->questions[question_to_send_id]->question;

//     Message *msg = create_msg(MSG_QUIZ_QUESTION, question_to_send, strlen(question_to_send));
//     send_msg(client->socket_fd, msg);
// }

// int verify_quiz_answer(char *answer, QuizQuestion *question)
// {

//     for (int a = 0; a < question->total_answers; a++)
//         if (strcasecmp(answer, question->answers[a]) == 0)
//             return 1;

//     return 0;
// }

// void handle_quiz_answer(Client *client, Message *msg, QuizzesInfo *quizzesInfo)
// {
//     char *user_answer = msg->payload;

//     RankingNode *current_ranking = client->client_rankings[client->current_quiz_id];
//     Quiz *playing_quiz = quizzesInfo->quizzes[client->current_quiz_id];
//     QuizQuestion *current_question = playing_quiz->questions[current_ranking->current_question];
//     Message *result_msg;
//     char *payload;

//     int correct_answer = verify_quiz_answer(user_answer, current_question);

//     if (correct_answer)
//     {
//         payload = "Risposta corretta";
//         current_ranking->score += 1;
//         update_ranking(current_ranking, playing_quiz);
//     }
//     else
//     {
//         payload = "Risposta errata";
//     }

//     result_msg = create_msg(MSG_QUIZ_RESULT, payload, strlen(payload));
//     send_msg(client->socket_fd, result_msg);

//     current_ranking->current_question += 1;
//     if (current_ranking->current_question == playing_quiz->total_questions)
//     {
//         current_ranking->is_quiz_completed = 1;
//         char *payload = "Hai terminato il quiz";
//         Message *msg = create_msg(MSG_INFO, payload, strlen(payload));
//         send_msg(client->socket_fd, msg);
//         client->state = SELECTING_QUIZ;
//         send_quiz_list(client, quizzesInfo);
//     }

//     else
//         send_quiz_question(client, playing_quiz);
// }

// void handle_quiz_selection(Client *client, Message *msg, QuizzesInfo *quizzesInfo)
// {

//     // nel caso in cui il payload non contenga
//     char *endptr;
//     int selected_quiz_number = (int)strtoul(msg->payload, &endptr, 10);

//     // Gestisco le possibili situazioni di errore

//     // Il quiz indicato non è disponibile
//     if (*endptr != '\0' || selected_quiz_number > quizzesInfo->total_quizzes)
//     {
//         char *message = "Quiz selezionato non valido";
//         Message *error_msg = create_msg(MSG_ERROR, message, strlen(message));
//         send_msg(client->socket_fd, error_msg);

//         send_quiz_list(client, quizzesInfo);
//         return;
//     }

//     // L'utente corrente ha già completato il quiz durante la sessione corrente
//     if (client->client_rankings[selected_quiz_number - 1] != NULL)
//     {
//         char *message = "Il quiz selezionato è già stato completato in questa sessione";
//         Message *error_msg = create_msg(MSG_ERROR, message, strlen(message));
//         send_msg(client->socket_fd, error_msg);

//         send_quiz_list(client, quizzesInfo);
//         return;
//     }
//     client->current_quiz_id = selected_quiz_number - 1;

//     // Inizializzo le informazioni relative al Ranking
//     Quiz *selected_quiz = quizzesInfo->quizzes[selected_quiz_number - 1];
//     RankingNode *new_node = create_ranking_node(client);
//     client->client_rankings[client->current_quiz_id] = new_node;

//     insert_ranking_node(selected_quiz, new_node);

//     client->state = PLAYING;

//     send_quiz_question(client, selected_quiz);
// }

// void handle_ranking_request(Client *client, QuizzesInfo *quizzesInfo)
// {
//     char *payload = "Ecco il tuo ranking";
//     Message *reply_msg = create_msg(MSG_INFO, payload, strlen(payload));
//     send_msg(client->socket_fd, reply_msg);

//     switch (client->state)
//     {
//     case PLAYING:
//         send_quiz_question(client, quizzesInfo->quizzes[client->current_quiz_id]);
//         break;
//     case SELECTING_QUIZ:
//         send_quiz_list(client, quizzesInfo);
//     default:
//         break;
//     }
// }

void handle_client(Client *client, Context *context)
{

    // int res = receive_msg(client->socket_fd, payload);
    //  if (res == 0)
    //  {
    //      printf("Il client ha chiuso la connessione\n");
    //      handle_client_disconnection(client, master, clientsInfo, quizzesInfo);
    //      free(received_msg);
    //      return;
    //  }
    //  else if (res == -1)
    //  {
    //      printf("Si è verificato un errore durante la ricezione di un messaggio\n");
    //      free(received_msg);
    //      return;
    //  }

    switch (client->state)
    {
    case LOGIN:
        handle_client_login(client, &context->clientsInfo);
        break;

    default:
        break;
    }

    // if (client->state == LOGIN)
    //     handle_client_nickname(client, received_msg, clientsInfo);
    // else if (received_msg->type == MSG_REQ_QUIZ_LIST && client->state == LOGGED_IN)
    //     send_quiz_list(client, quizzesInfo);
    // else if (received_msg->type == MSG_QUIZ_SELECT && client->state == SELECTING_QUIZ)
    //     handle_quiz_selection(client, received_msg, quizzesInfo);

    // else if (received_msg->type == MSG_QUIZ_ANSWER && client->state == PLAYING)
    //     handle_quiz_answer(client, received_msg, quizzesInfo);

    // else if (received_msg->type == MSG_REQ_RANKING)
    //     handle_ranking_request(client, quizzesInfo);
}