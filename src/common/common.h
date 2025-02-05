#ifndef COMMON_H
#define COMMON_H

#include "stddef.h"

/**
 * @brief Enumerazione che definisce i tipi di messaggi scambiati tra client e server
 *
 * Questa enumerazione rappresenta i diversi tipi di messaggi che possono essere
 * inviati e ricevuti durante la comunicazione tra il client e il server nel sistema di quiz.
 */
typedef enum MessageType
{
    MSG_REQ_NICKNAME,  /**< Messaggio inviato dal server per richiedere al client il nickname */
    MSG_SET_NICKNAME,  /**< Messaggio inviato dal client per settare il nickname */
    MSG_OK_NICKNAME,   /**< Messaggio inviato dal server per indicare che il nickname scelto è corretto */
    MSG_REQ_QUIZ_LIST, /**< Messaggio inviato dal client per richiedere la lista dei quiz */
    MSG_RES_QUIZ_LIST, /**< Messaggio inviato dal server per inoltrare la lista dei quiz [BINARY PROTOCOL] */
    MSG_QUIZ_SELECT,   /**< Messaggio inviato dal client per selezionare il quiz */
    MSG_QUIZ_SELECTED, /**< Messaggio inviato dal server al client per confermare il quiz selezionato */
    MSG_QUIZ_QUESTION, /**< Messaggio inviato dal server per inoltrare la domanda del quiz */
    MSG_QUIZ_ANSWER,   /**< Messaggio inviato dal client con la risposta alla domanda */
    MSG_REQ_RANKING,   /**< Messaggio inviato dal client al server per richiedere la classifica */
    MSG_RES_RANKING,   /**< Messaggio inviato dal server al client per la classifica [BINARY PROTOCOL] */
    MSG_DISCONNECT,    /**< Messaggio inviato dal client o dal server per indicare la disconnessione */
    MSG_INFO           /**< Messaggio inviato dal server con un messaggio informativo per il client */
} MessageType;

/**
 * @brief Struttura che rappresenta un messaggio scambiato tra client e server
 *
 * In particolare il payload viene allocato nello heap in base a payload_length
 */
typedef struct Message
{
    MessageType type;   /**< Tipo del messaggio */
    int payload_length; /**< Dimensione del payload in byte */
    char *payload;      /**< Puntatore ai dati del payload del messaggio */
} Message;

void handle_malloc_error(void *ptr, const char *error_string);
int receive_msg(int client_fd, Message *msg);
int send_msg(int client_fd, MessageType type, char *payload, size_t payload_len);
int get_console_input(char *buffer, int buffer_size);

#endif