#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include "../constants.h"

#define MAX_PAYLOAD_SIZE 256

typedef enum
{
    LOGIN,
    LOGGED_IN,
    SELECTING_QUIZ,
    PLAYING
} ClientState;

typedef struct Client
{
    int socket_fd;
    char *nickname;
    ClientState state;
    struct RankingNode **client_rankings;
    int current_quiz_id;

} Client;

typedef struct ClientsInfo
{
    Client clients[MAX_CLIENTS];
    int connected_clients;
    int max_fd;

} ClientsInfo;

typedef struct QuizQuestion
{
    char *question;
    char **answers;
    int total_answers;
} QuizQuestion;

typedef struct Quiz
{
    char *name;
    QuizQuestion **questions;
    int total_questions;
    struct RankingNode *ranking_head;
    struct RankingNode *ranking_tail;
} Quiz;

typedef struct QuizzesInfo
{
    Quiz **quizzes;
    int total_quizzes;
} QuizzesInfo;

typedef struct RankingNode
{
    Client *client;
    int score;
    int is_quiz_completed;
    int current_question;
    struct RankingNode *prev_node;
    struct RankingNode *next_node;

} RankingNode;
typedef enum
{
    MSG_SET_NICKNAME,  // messaggio inviato dal client per settare il nickname
    MSG_REQ_NICKNAME,  // messaggio inviato dal server per richiedere al client il nickaname
    MSG_OK_NICKNAME,   // messaggio inviato dal server per indicare che il nickname scelto è corretto
    MSG_REQ_QUIZ_LIST, // messaggio inviato dal client per richiedere la lista dei quiz
    MSG_RES_QUIZ_LIST, // messaggio inviato dal server per rispondere alla richiesta del client
    MSG_QUIZ_SELECT,   // messaggio inviato dal client per selezionare il quiz
    MSG_QUIZ_QUESTION, // messaggio inviato dal server con la domanda
    MSG_QUIZ_ANSWER,   // messaggio inviato dal client con la risposta alla domanda
    MSG_QUIZ_RESULT,   // messaggio inviato dal server con il risultato della risposta
    MSG_ERROR,         // messaggio inviato dal server o dal client per indicare che si è verificato un errore
    MSG_INFO,          // messaggio inviato dal server con un messaggio informativo per il client
    MSG_REQ_RANKING,   // messaggio inviato dal client al server per richiedere la classifica
    MSG_RES_RANKING    // messaggio inviato dal server al client per la classifica
} MessageType;

typedef struct
{
    MessageType type;
    int payload_length;
    char payload[MAX_PAYLOAD_SIZE];
} Message;

// Client list

void handle_new_client_connection(ClientsInfo *clientsInfo, fd_set *master, int server_fd);
void handle_client(Client *client, ClientsInfo *clientsInfo, QuizzesInfo *quizzesInfo, fd_set *master);
void init_clients_info(ClientsInfo *clientsInfo, QuizzesInfo *quizzesInfo);

// Quiz

Quiz *load_quiz_from_file(const char *file_path);
int load_quizzes_from_directory(const char *directory_path, QuizzesInfo *quizzesInfo);

// Dashboard

void show_dashboard(QuizzesInfo *quizzesInfo, ClientsInfo *clientsInfo, int score);

// Ranking

RankingNode *create_ranking_node(Client *client);
void insert_ranking_node(Quiz *quiz, RankingNode *node);
void list_rankings(Quiz *quiz);
void list_completed_rankings(Quiz *quiz);
void update_ranking(RankingNode *node, Quiz *quiz);
void remove_ranking(RankingNode *node, Quiz *quiz);

#endif // UTILS_H