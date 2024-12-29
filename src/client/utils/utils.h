#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_PAYLOAD_SIZE 256

typedef enum
{
    MSG_SET_NICKNAME,      // messaggio inviato dal client per settare il nickname
    MSG_REQ_NICKNAME,      // messaggio inviato dal server per richiedere al client il nickaname
    MSG_OK_NICKNAME,       // messaggio inviato dal server per indicare che il nickname scelto è corretto
    MSG_REQ_QUIZ_LIST,     // messaggio inviato dal client per richiedere la lista dei quiz
    MSG_RES_QUIZ_LIST,     // messaggio inviato dal server per rispondere alla richiesta del client
    MSG_QUIZ_SELECT,       // messaggio inviato dal client per selezionare il quiz
    MSG_QUIZ_SELECT_ERROR, // messaggio inviato dal server per indicare un errore nella selezione del quiz
    MSG_QUIZ_QUESTION,     // messaggio inviato dal server con la domanda
    MSG_QUIZ_ANSWER,       // messaggio inviato dal client con la risposta alla domanda
    MSG_QUIZ_RESULT,       // messaggio inviato dal server con il risultato della risposta
    MSG_ERROR              // messaggio inviato dal server o dal client per indicare che si è verificato un errore
} MessageType;

typedef struct
{
    MessageType type;
    int payload_length;
    char payload[MAX_PAYLOAD_SIZE];
} Message;

void show_menu();
void show_quiz_list(char **quizzes, int total_quizzes);

int receive_msg(int client_fd, Message *msg);

void handle_nickname_selection(int server_fd, Message *msg);
void handle_quiz_selection(int server_fd, Message *msg);
void request_available_quizzes(int server_fd);
void deserialize_quiz_list(Message *msg, char ***quizzes, int *total_quizzes);

#endif // UTILS_H