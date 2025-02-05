#include "utils.h"
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "../../common/common.h"
#include "../../common/params.h"

/**
 * @brief Gestisce la selezione del nickname da parte dell'utente
 *
 * Questa funzione risponde all'arrivo di un messaggio di tipo MSG_REQ_NICKNAME, tramite il quale il server richiede all'utente di inserire un nickname.
 * Questa funzione gestisce grazie a get_console_input i casi in cui il valore inserito superi la dimensione del buffer o sia vuoto e invia il nickname selezionato
 * al server tramite un messaggio di tipo MSG_SET_NICKNAME
 *
 * @param server_fd file descriptor del socket del server
 * @param msg puntatore al messaggio che contiene il messaggio da visualizzare
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
 * @brief Gestisce la selezione del nickname da parte dell'utente
 *
 * Questa funzione risponde all'arrivo di un messaggio di tipo MSG_OK_NICKNAME, tramite il quale il server comunica all'utente che il nickname è stato accettato.
 * Si risponde inviando una richiesta al server per ottenere la lista dei quiz disponibili tramite un messaggio di tipo MSG_REQ_QUIZ_LIST
 *
 * @param server_fd file descriptor del socket del server
 */
void request_available_quizzes(int server_fd)
{
    send_msg(server_fd, MSG_REQ_QUIZ_LIST, "", 0);
}

/**
 * @brief Gestisce la deserializzazione e la visualizzazione della lista dei quiz disponibili
 *
 * Questa funzione si occupa di effettuare la deserializzazione utilizzando binary protocol
 * della lista dei quiz disponibili ricevita dal server.
 *
 * In particolare utilizzo la funzione ntohl per confertire da network byte order a host byte order
 * e utilizzo i tipi standardizzati uint32_t per garantire la portabilità.
 *
 * I dati relativi ai quiz disponibili vengono ricevuti secondo questo formato binary
 * (numero quizzes) [(lunghezza nome)(nome)] [(lunghezza nome)(nome)] [...]
 * dove le parentesi danno un'idea del livello di annidamento e chiaramente non sono incluse nei dati inviati.
 *
 * @param msg puntatore al messaggio nel cui payload si trova la lista dei quiz serializzata
 */
void display_quiz_list(Message *msg)
{
    // inizializzo le strutture dati
    char *pointer = msg->payload;
    size_t string_len;
    uint32_t net_strings_num, net_string_len, total_quizzes;
    memcpy(&net_strings_num, pointer, sizeof(uint32_t));
    pointer += sizeof(uint32_t);
    // prelievo e converto dal buffer il numero totale dei quiz e effettuo un loop
    total_quizzes = ntohl(net_strings_num);

    printf("\nQuiz disponibili\n");
    printf("+++++++++++++++++++++++++++\n");

    for (uint32_t i = 0; i < total_quizzes; i++)
    {
        memcpy(&net_string_len, pointer, sizeof(uint32_t));
        string_len = ntohl(net_string_len);
        pointer += sizeof(uint32_t);

        printf("%u - %.*s\n", (unsigned)(i + 1), (int)string_len, pointer);

        pointer += string_len;
    }

    printf("+++++++++++++++++++++++++++\n");
}

/**
 * @brief Gestisce la deserializzazione e la visualizzazione della classifica
 *
 * Questa funzione si occupa di effettuare la deserializzazione utilizzando binary protocol
 * della classifica per ogni quiz ricevuta dal server.
 *
 * In particolare utilizzo la funzione ntohl per confertire da network byte order a host byte order
 * e utilizzo i tipi standardizzati uint32_t per garantire la portabilità.
 *
 * I dati relativi al ranking dei quiz vengono ricevuti secondo questo formato binary protocol
 * (numero quizzes)  {(numero utenti partecipanti al quiz) [(lunghezza nome) (nome) (score)]} {...}
 * dove le parentesi danno un'idea del livello di annidamento e chiaramente non sono incluse nei dati inviati.
 *
 * @param msg puntatore al messaggio nel cui payload si trova la classifica serializzata
 */
void handle_rankings(Message *msg)
{
    char *pointer = msg->payload;
    uint32_t clients_per_quiz, quizzes_num, client_score, string_len;
    uint32_t net_string_len, net_client_score, net_clients_per_quiz, net_quizzes_num;
    // prelevo il numero totale dei quiz
    memcpy(&net_quizzes_num, pointer, sizeof(uint32_t));
    quizzes_num = ntohl(net_quizzes_num);
    pointer += sizeof(uint32_t);

    for (uint32_t i = 0; i < quizzes_num; i++)
    {
        memcpy(&net_clients_per_quiz, pointer, sizeof(uint32_t));
        clients_per_quiz = ntohl(net_clients_per_quiz);
        pointer += sizeof(uint32_t);
        // prelevo il numero dei client che stanno partecipando al quiz
        printf("\nPunteggio Tema %d\n", i + 1);
        for (uint32_t j = 0; j < clients_per_quiz; j++)
        {
            // prelevo la lunghezza del nickname per il client
            memcpy(&net_string_len, pointer, sizeof(uint32_t));
            string_len = ntohl(net_string_len);
            pointer += sizeof(uint32_t);

            // prelevo lo score del client
            memcpy(&net_client_score, pointer + string_len, sizeof(uint32_t));
            client_score = ntohl(net_client_score);
            // stampo a schermo il nickname e lo score
            printf("- %.*s %d\n", string_len, pointer, client_score);
            pointer += string_len + sizeof(uint32_t);
        }
    }
}
/**
 * @brief Gestisce la selezione del quiz da parte del client
 *
 * Questa funzione risponde all'arrivo di un messaggio di tipo MSG_RES_QUIZ_LIST, tramite il quale il server comunica all'utente la lista dei quiz in formato serializzato,
 * della cui deserializzazione e stampa a schermo si occupa la funzione handle_quiz_selection
 *
 * La funzione permette all'utente di effettuare una scelta, in particolare il comportamento cambia a seconda del valore digitato dal client
 *  - ENDQUIZ: invio di un messaggio MSG_DISCONNECT che porta il server a deallocare le strutture del client e chiudere la connessione
 *  - SHOWSCORE: invio di un messaggio MSG_REQ_RANKING che porta il server a rispondere con un messaggio di tipo MSG_RES_RANKING e dunque all'invio della classifica
 *  - ELSE: invio di un messaggio MSG_QUIZ_SELECT con payload dato dal valore inserito che permette al client di selezionare il quiz a cui vuole partecipare
 *
 *
 * @param server_fd file descriptor del socket del server
 */
void handle_quiz_selection(int server_fd, Message *msg)
{
    display_quiz_list(msg);

    char answer[DEFAULT_PAYLOAD_SIZE];
    do
        printf("La tua scelta: ");
    while (get_console_input(answer, sizeof(answer)) == -1);

    if (strcmp(answer, ENDQUIZ) == 0)
        send_msg(server_fd, MSG_DISCONNECT, "", 0);
    else if (strcmp(answer, SHOWSCORE) == 0)
        send_msg(server_fd, MSG_REQ_RANKING, "", 0);
    else
        send_msg(server_fd, MSG_QUIZ_SELECT, answer, strlen(answer));
}
/**
 * @brief Gestisce la ricezione di un messaggio informativo
 *
 * Questa funzione risponde all'arrivo di un messaggio di tipo MSG_INFO
 * Il client stampa a schermo il messaggio
 *
 * @param msg puntatore al messaggio ricevuto
 */
void handle_message(Message *msg)
{
    printf("\n%s\n", msg->payload);
}
/**
 * @brief Gestisce la risposta ad una domanda del quiz da parte del client
 *
 * Questa funzione risponde all'arrivo di un messaggio di tipo MSG_QUIZ_QUESTION, tramite il quale il server comunica all'utente la domanda del quiz a cui rispondere
 *
 * La funzione permette all'utente di effettuare una scelta, in particolare il comportamento cambia a seconda del valore digitato dal client
 *  - ENDQUIZ: invio di un messaggio MSG_DISCONNECT che porta il server a deallocare le strutture del client e chiudere la connessione
 *  - SHOWSCORE: invio di un messaggio MSG_REQ_RANKING che porta il server a rispondere con un messaggio di tipo MSG_RES_RANKING e dunque all'invio della classifica
 *  - ELSE: invio di un messaggio MSG_QUIZ_ANSWER con payload dato dal valore inserito che permette al client di rispondere alla domanda
 *
 *
 * @param server_fd file descriptor del socket del server
 * @param msg puntatore al messaggio ricevuto
 */
void handle_quiz_question(int server_fd, Message *msg)
{
    char answer[DEFAULT_PAYLOAD_SIZE];
    printf("\n%s\n", msg->payload);
    do
        printf("Risposta: ");
    while (get_console_input(answer, sizeof(answer)) == -1);

    if (strcmp(answer, ENDQUIZ) == 0)
        send_msg(server_fd, MSG_DISCONNECT, "", 0);
    else if (strcmp(answer, SHOWSCORE) == 0)
        send_msg(server_fd, MSG_REQ_RANKING, "", 0);
    else
        send_msg(server_fd, MSG_QUIZ_ANSWER, answer, strlen(answer));
}
