#ifndef COMMON_H
#define COMMON_H

#include "stddef.h"

#define DEFAULT_PAYLOAD_SIZE 256

typedef enum MessageType
{
    MSG_SET_NICKNAME,  // messaggio inviato dal client per settare il nickname
    MSG_REQ_NICKNAME,  // messaggio inviato dal server per richiedere al client il nickname
    MSG_OK_NICKNAME,   // messaggio inviato dal server per indicare che il nickname scelto è corretto
    MSG_REQ_QUIZ_LIST, // messaggio inviato dal client per richiedere la lista dei quiz
    MSG_RES_QUIZ_LIST, // messaggio inviato dal server per rispondere alla richiesta del client
    MSG_QUIZ_SELECT,   // messaggio inviato dal client per selezionare il quiz
    MSG_QUIZ_SELECTED, // messaggio inviato dal server al client per confermare il quiz selezionato
    MSG_QUIZ_QUESTION, // messaggio inviato dal server con la domanda
    MSG_QUIZ_ANSWER,   // messaggio inviato dal client con la risposta alla domanda
    MSG_ERROR,         // messaggio inviato dal server per indicare che si è verificato un errore
    MSG_INFO,          // messaggio inviato dal server con un messaggio informativo per il client
    MSG_REQ_RANKING,   // messaggio inviato dal client al server per richiedere la classifica
    MSG_RES_RANKING,   // messaggio inviato dal server al client per la classifica
    MSG_DISCONNECT     // messaggio inviato dal client o dal server per indicare la disconnessione
} MessageType;

typedef struct Message
{
    MessageType type;
    int payload_length;
    char *payload;
} Message;

Message *create_msg(MessageType type, char *payload, size_t payload_len);
int receive_msg(int client_fd, Message *msg);
void send_msg(int client_fd, Message *msg);

void handle_malloc_error(void *ptr, const char *error_string);

#endif