#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "../../common/common.h"
#include "utils.h"
#include <sys/select.h>
#include "errno.h"

/**
 * @brief Gestisce l'inizializzazione delle informazioni relative ai clients che si possono connettere al sistema
 *
 * @param clientsInfo puntatore alla struttura con le informazioni sui clients
 * @return struttura di tipo Quiz relativa al file file_path
 */
void init_clients_info(ClientsInfo *clientsInfo)
{
    clientsInfo->connected_clients = 0;
    clientsInfo->clients_head = clientsInfo->clients_tail = NULL;
    clientsInfo->max_fd = 0;
}
/**
 * @brief Crea un nuovo nodo di tipo client
 *
 * Questa funzione si occupa di inizializzare un nuovo client che si è connesso al sistema
 * In particolare si inizializza l'array contente tutti gli oggetti RankingNode relativi alle partite del client
 *
 * @param client_fd file descriptor del socket del client
 * @param quizzesInfo puntatore alla struttura che contiene le informazioni sui quiz
 * @return struttura di tipo Client relativa al nuovo client connesso
 */
Client *create_client_node(int client_fd, QuizzesInfo *quizzesInfo)
{
    Client *new_client = (Client *)malloc(sizeof(Client));
    handle_malloc_error(new_client, "Errore nell'allocazione della memoria per il nuovo client");
    new_client->client_rankings = NULL;
    new_client->nickname = NULL;
    new_client->next_node = new_client->prev_node = NULL;
    new_client->current_quiz_id = -1;
    new_client->socket_fd = client_fd;
    new_client->state = LOGIN;
    new_client->client_rankings = malloc(quizzesInfo->total_quizzes * sizeof(RankingNode *));
    handle_malloc_error(new_client->client_rankings, "Errore nell'allocazione della memoria per i ranking del nuovo client");
    memset(new_client->client_rankings, 0, quizzesInfo->total_quizzes * sizeof(RankingNode *));
    return new_client;
}
/**
 * @brief Aggiunge una struttura di tipo Client alla lista dei client connessi
 *
 * Questa funzione si occupa di inserire il nodo client in coda alla lista doppiamente concatenata dei client attivi sul sistema
 *
 * @param node puntatore alla struttura Client da inserire
 * @param clientsInfo puntatore alla struttura che contiene le informazioni sui clients
 */
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
/**
 * @brief Rimuove e dealloca una struttura di tipo Client alla lista dei client connessi
 *
 * @param node puntatore alla struttura Client da rimuovere e deallocare
 * @param clientsInfo puntatore alla struttura che contiene le informazioni sui clients
 */
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
    free(node->client_rankings);
    free(node);
}
/**
 * @brief Rimuove e dealloca tutti i clients all'interno della lista dei clients connessi al sistema
 *
 * @param clientsInfo puntatore alla struttura che contiene le informazioni sui clients
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
 * @brief Invia un messaggio al client per richiedere l'username
 *
 * Questa funzione si occupa di inviare al client con socket client_fd un messaggio di tipo MSG_REQ_NICKNAME
 *
 * @param clientsInfo puntatore alla struttura che contiene le informazioni sui clients
 */
void request_client_nickname(int client_fd)
{
    char *message = "Scegli un nickname (deve essere univoco): ";
    Message *msg = create_msg(MSG_REQ_NICKNAME, message, strlen(message));
    send_msg(client_fd, msg);
}

/**
 * @brief Gestisce la connessione di un nuovo client al sistema
 *
 * Questa funzione viene invocata quando vi è una nuova connessione sul socket del server
 * e si occupa di accettare la connessione e di creare tutte le strutture dati necessarie
 * per gestire il nuovo client
 *
 * @param context puntatore alla struttura che contiene le informazioni sul contesto del servizio
 */
void handle_new_client_connection(Context *context)
{
    struct sockaddr_in client_address;
    int client_fd;
    socklen_t address_size = sizeof(client_address);
    // accettiamo la connessione
    client_fd = accept(context->server_fd, (struct sockaddr *)&client_address, &address_size);
    if (client_fd == -1)
    {
        if (errno == EINTR)
            return;
        else
            exit(EXIT_FAILURE);
    }
    // aggiungo il file descriptor al set master
    FD_SET(client_fd, &context->masterfds);
    // aggiorno max_fd
    if (client_fd > context->clientsInfo.max_fd)
        context->clientsInfo.max_fd = client_fd;

    // creo il nodo client e lo aggiungo in coda alla lista
    Client *client = create_client_node(client_fd, &context->quizzesInfo);
    add_client(client, &context->clientsInfo);

    // invio il messaggio di richiesta dell'username
    request_client_nickname(client_fd);
}

/**
 * @brief Invia al client la lista dei quiz disponibili
 *
 * Questa funzione si occupa di effettuare la serializzazione utilizzando binary protocol
 * della lista dei quiz disponibili e di inviarli al client
 *
 * In particolare utilizzo la funzione htonl per confertire da host byte order a network byte order
 * e utilizzo i tipi standardizzati uint32_t per garantire la portabilità
 *
 * I dati relativi ai quiz disponibili verranno serializzati secondo questo formato binary
 * [numero quizzes][lunghezza nome][nome][lunghezza nome][nome]...
 *
 * @param client puntatore al client a cui inviare la lista
 * @param quizzesInfo puntatore alla struttura che contiene le informazioni sui quiz
 */
void send_quiz_list(Client *client, QuizzesInfo *quizzesInfo)
{
    // creiamo un payload per il nostro pacchetto di dimensione standard
    size_t buffer_size = DEFAULT_PAYLOAD_SIZE;
    char *payload = (char *)malloc(buffer_size);
    handle_malloc_error(payload, "Errore nell'allocazione del payload");

    char *pointer = payload;
    size_t string_len;
    uint32_t net_string_len;

    // inserisco nel buffer il numero totale di quiz disponibili
    uint32_t net_strings_num = htonl(quizzesInfo->total_quizzes);
    memcpy(pointer, &net_strings_num, sizeof(uint32_t));
    pointer += sizeof(uint32_t);

    Quiz *quiz;

    for (int i = 0; i < quizzesInfo->total_quizzes; i++)
    {
        quiz = quizzesInfo->quizzes[i];
        string_len = strlen(quiz->name);
        net_string_len = htonl(string_len);

        // gestisco la situazione in cui il buffer payload precedentemente allocato
        // non è abbastanza capiente
        if (pointer + sizeof(uint32_t) + string_len > payload + buffer_size)
        {
            // nel caso in cui la dimensione del buffer non sia abbastanza capiente lo rialloco
            size_t new_size = buffer_size * 2;
            char *new_payload = (char *)realloc(payload, new_size);
            handle_malloc_error(new_payload, "Errore nelle riallocazione di payload");

            pointer = new_payload + (pointer - payload);
            payload = new_payload;
            buffer_size = new_size;
        }

        // copio nel buffer la dimensione della stringa
        memcpy(pointer, &net_string_len, sizeof(uint32_t));
        pointer += sizeof(uint32_t);

        // copio nel buffer la stringa
        memcpy(pointer, quiz->name, string_len);
        pointer += string_len;
    }

    int payload_size = pointer - payload;

    client->state = SELECTING_QUIZ;

    // invio il messaggio serializzato
    Message *msg = create_msg(MSG_RES_QUIZ_LIST, payload, payload_size);
    send_msg(client->socket_fd, msg);
    free(payload);
}

/**
 * @brief Controlla la validità del nickname fornito dal client
 *
 * Questa funzione viene invocata in seguito alla ricezione di un messaggio di tipo MSG_SET_NICKNAME dal client e
 * si occupa di controllare se questo è valido o meno controllando la sua unicità
 *
 * @param client puntatore al client che ha inviato il nickname
 * @param received_msg puntatore al messaggio ricevuto
 * @param clientsInfo puntatore alla struttura con le informazioni sui clients
 */
void handle_client_nickname(Client *client, Message *received_msg, ClientsInfo *clientsInfo)
{
    char *selected_nickname = received_msg->payload;

    int found = 0;

    // eseguo una ricerca lineare tra i client a cui è già stato associato un nickname
    Client *current_client = clientsInfo->clients_head;
    while (current_client != NULL)
    {
        if (current_client->nickname && strcmp(selected_nickname, current_client->nickname) == 0)
        {
            found = 1;
            break;
        }
        current_client = current_client->next_node;
    }
    if (found)
    {
        // se il nickname è già utilizzato invio un messaggio che indica la situazione
        char *message = "Nickname già in uso";
        Message *error_msg = create_msg(MSG_INFO, message, strlen(message));
        send_msg(client->socket_fd, error_msg);
        // richiedo nuovamente al client di fornire un nickname valido
        request_client_nickname(client->socket_fd);
        return;
    }

    client->state = LOGGED_IN;
    clientsInfo->connected_clients += 1;
    client->nickname = malloc(strlen(selected_nickname) + 1);
    handle_malloc_error(client->nickname, "Errore nell'allocazione della memoria per il nickname del client");
    strcpy(client->nickname, selected_nickname);

    // invio di un messaggio di nickname corretto al client
    Message *msg = create_msg(MSG_OK_NICKNAME, "", 0);
    send_msg(client->socket_fd, msg);
}
/**
 * @brief Gestisce la disconnessione di un client
 *
 * Questa funzione va a gestire la disconnessione di un client che può avvenire esplicitamente tramite l'invio di un messaggio MSG_DISCONNECT
 * oppure implicitamente attraverso la chiusura diretta del socket
 *
 * @param client puntatore al client che si vuole disconnettere
 * @param context puntatore alla struttura che contiene le informazioni sul contesto del servizio
 */
void handle_client_disconnection(Client *client, Context *context)
{
    // rimuovo il client dal set dei descrittori attivi e chiudo il socket
    FD_CLR(client->socket_fd, &context->masterfds);
    close(client->socket_fd);

    // rimuovo tutte le classifiche del client
    for (int i = 0; i < context->quizzesInfo.total_quizzes; i++)
        remove_ranking(client->client_rankings[i], context->quizzesInfo.quizzes[i]);

    // aggiorno max_fd
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

    context->clientsInfo.connected_clients--;
    remove_client(client, &context->clientsInfo);
}
/**
 * @brief Invia la domanda corrente al client
 *
 * Questa funzione invia al client un messaggio di tipo MSG_QUIZ_QUESTION che contiene la successiva domanda del quiz a cui
 * sta partecipando
 *
 * @param client puntatore al client a cui inviare la domanda
 * @param quiz puntatore alla struttura che contiene le informazioni sul quiz
 */
void send_quiz_question(Client *client, Quiz *quiz)
{
    int question_to_send_id = client->client_rankings[client->current_quiz_id]->current_question;
    if (question_to_send_id >= quiz->total_questions)
        return;
    // si seleziona la domanda corretta da inviare e si invia al client
    char *question_to_send = quiz->questions[question_to_send_id]->question;

    Message *msg = create_msg(MSG_QUIZ_QUESTION, question_to_send, strlen(question_to_send));
    send_msg(client->socket_fd, msg);
}
/**
 * @brief Verifica la risposta del client
 *
 * Questa funzione controlla se la risposta si trova tra le risposte possibili per quella domanda
 * Si noti che le risposte sono case insensitive
 *
 * @param answer puntatore alla risposta del client
 * @param quiz puntatore alla struttura che contiene la domanda del quiz
 */
bool verify_quiz_answer(char *answer, QuizQuestion *question)
{
    for (int a = 0; a < question->total_answers; a++)
        if (strcasecmp(answer, question->answers[a]) == 0)
            return true;
    return false;
}

void handle_quiz_answer(Client *client, Message *msg, QuizzesInfo *quizzesInfo)
{
    char *user_answer = msg->payload;

    RankingNode *current_ranking = client->client_rankings[client->current_quiz_id];
    Quiz *playing_quiz = quizzesInfo->quizzes[client->current_quiz_id];
    QuizQuestion *current_question = playing_quiz->questions[current_ranking->current_question];
    Message *result_msg;
    char *payload;

    int correct_answer = verify_quiz_answer(user_answer, current_question);

    if (correct_answer)
    {
        payload = "Risposta corretta";
        current_ranking->score += 1;
        update_ranking(current_ranking, playing_quiz);
    }
    else
    {
        payload = "Risposta errata";
    }

    result_msg = create_msg(MSG_INFO, payload, strlen(payload));
    send_msg(client->socket_fd, result_msg);

    current_ranking->current_question += 1;
    if (current_ranking->current_question == playing_quiz->total_questions)
    {
        current_ranking->is_quiz_completed = 1;
        char *payload = "Hai terminato il quiz";
        Message *msg = create_msg(MSG_INFO, payload, strlen(payload));
        send_msg(client->socket_fd, msg);
        client->state = SELECTING_QUIZ;
        send_quiz_list(client, quizzesInfo);
    }

    else
        send_quiz_question(client, playing_quiz);
}

void handle_quiz_selection(Client *client, Message *msg, QuizzesInfo *quizzesInfo)
{

    // nel caso in cui il payload non contenga
    char *endptr;
    int selected_quiz_number = (int)strtoul(msg->payload, &endptr, 10);

    // Gestisco le possibili situazioni di errore

    // Il quiz indicato non è disponibile
    if (*endptr != '\0' || selected_quiz_number > quizzesInfo->total_quizzes)
    {
        char *message = "Quiz selezionato non valido";
        Message *error_msg = create_msg(MSG_INFO, message, strlen(message));
        send_msg(client->socket_fd, error_msg);

        send_quiz_list(client, quizzesInfo);
        return;
    }

    // L'utente corrente ha già completato il quiz durante la sessione corrente
    if (client->client_rankings[selected_quiz_number - 1] != NULL)
    {
        char *message = "Il quiz selezionato è già stato completato in questa sessione";
        Message *error_msg = create_msg(MSG_INFO, message, strlen(message));
        send_msg(client->socket_fd, error_msg);

        send_quiz_list(client, quizzesInfo);
        return;
    }
    client->current_quiz_id = selected_quiz_number - 1;

    // Inizializzo le informazioni relative al Ranking
    Quiz *selected_quiz = quizzesInfo->quizzes[selected_quiz_number - 1];
    RankingNode *new_node = create_ranking_node(client);
    client->client_rankings[client->current_quiz_id] = new_node;

    insert_ranking_node(selected_quiz, new_node);

    client->state = PLAYING;

    Message *selected_quiz_msg = create_msg(MSG_QUIZ_SELECTED, selected_quiz->name, strlen(selected_quiz->name));
    send_msg(client->socket_fd, selected_quiz_msg);

    send_quiz_question(client, selected_quiz);
}

void handle_ranking_request(Client *client, QuizzesInfo *quizzesInfo)
{
    size_t buffer_size = DEFAULT_PAYLOAD_SIZE;
    char *payload = (char *)malloc(buffer_size);
    handle_malloc_error(payload, "Errore nell'allocazione del payload");

    // [numero quizzes] [[numero utenti partecipanti al quiz] [[lunghezza nome] [nome] [score]]]

    char *pointer = payload;
    int clients_per_quiz;
    size_t string_len;
    uint32_t net_string_len, net_client_score, net_clients_per_quiz;
    char *quiz_total_clients_pointer;

    // inserisco il numero totale di quizzes per cui dobbiamo stampare la classifica
    uint32_t net_quizzes_num = htonl(quizzesInfo->total_quizzes);
    memcpy(pointer, &net_quizzes_num, sizeof(uint32_t));
    pointer += sizeof(uint32_t);

    // per ogni quiz inserisco in ordine: numero di utenti che hanno partecipato,
    // lunghezza di ogni nome utente, nome utente e punteggio
    for (int i = 0; i < quizzesInfo->total_quizzes; i++)
    {
        Quiz *quiz = quizzesInfo->quizzes[i];
        // memorizzo la posizione dove andare ad inserire al termine del conteggio il numero totale
        // di utenti che sono in classifica per il quiz corrente
        quiz_total_clients_pointer = pointer;
        pointer += sizeof(uint32_t);

        clients_per_quiz = 0;
        RankingNode *current = quiz->ranking_head;
        while (current)
        {
            clients_per_quiz += 1;
            string_len = strlen(current->client->nickname);
            net_string_len = htonl(string_len);

            // gestisco la situazine in cui il buffer allocato non è abbastanza grande da contenere
            // le informazioni
            if (pointer + sizeof(uint32_t) + string_len + sizeof(uint32_t) > payload + buffer_size)
            {
                size_t new_size = buffer_size * 2;
                char *new_payload = (char *)realloc(payload, new_size);
                handle_malloc_error(new_payload, "Errore nella riallocazione della memoria per il payload");

                pointer = new_payload + (pointer - payload);
                payload = new_payload;
                buffer_size = new_size;
            }

            // inserisco la lunghezza del nickname del client corrent
            memcpy(pointer, &net_string_len, sizeof(uint32_t));
            pointer += sizeof(uint32_t);

            // inserisco il client nickname corrente
            memcpy(pointer, current->client->nickname, string_len);
            pointer += string_len;

            // inserisco lo score del client corrente
            net_client_score = htonl(current->score);
            memcpy(pointer, &net_client_score, sizeof(uint32_t));
            pointer += sizeof(uint32_t);
            current = current->next_node;
        }
        // a questo punto inserisco il numero totale di client in classifica per il quiz corrente
        // nella posizione del buffer salvata precedentemente
        net_clients_per_quiz = htonl(clients_per_quiz);
        memcpy(quiz_total_clients_pointer, &net_clients_per_quiz, sizeof(uint32_t));
    }

    int payload_size = pointer - payload;

    Message *reply_msg = create_msg(MSG_RES_RANKING, payload, payload_size);
    send_msg(client->socket_fd, reply_msg);

    free(payload);

    switch (client->state)
    {
    case PLAYING:
        send_quiz_question(client, quizzesInfo->quizzes[client->current_quiz_id]);
        break;
    case SELECTING_QUIZ:
        send_quiz_list(client, quizzesInfo);
        break;
    default:
        break;
    }
}

void handle_client(Client *client, Context *context)
{
    Message *received_msg = (Message *)malloc(sizeof(Message));
    int res = receive_msg(client->socket_fd, received_msg);
    if (res == 0)
    {
        printf("Il client ha chiuso la connessione in modo brusco\n");
        handle_client_disconnection(client, context);
        return;
    }
    else if (res == -1)
    {
        printf("Si è verificato un errore durante la ricezione di un messaggio\n");
        exit(EXIT_FAILURE);
    }

    switch (received_msg->type)
    {
    case MSG_SET_NICKNAME:
        handle_client_nickname(client, received_msg, &context->clientsInfo);
        break;
    case MSG_REQ_QUIZ_LIST:
        send_quiz_list(client, &context->quizzesInfo);
        break;
    case MSG_QUIZ_SELECT:
        handle_quiz_selection(client, received_msg, &context->quizzesInfo);
        break;
    case MSG_QUIZ_ANSWER:
        handle_quiz_answer(client, received_msg, &context->quizzesInfo);
        break;
    case MSG_REQ_RANKING:
        handle_ranking_request(client, &context->quizzesInfo);
        break;
    case MSG_DISCONNECT:
        handle_client_disconnection(client, context);
        break;
    default:
        break;
    }
    free(received_msg->payload);
    free(received_msg);
}