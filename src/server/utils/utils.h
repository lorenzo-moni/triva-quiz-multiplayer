#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include "../../common/common.h"

typedef enum ClientState
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
    struct Client *prev_node;
    struct Client *next_node;

} Client;

typedef struct ClientsInfo
{
    struct Client *clients_head;
    struct Client *clients_tail;
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

typedef struct Context
{
    ClientsInfo clientsInfo;
    QuizzesInfo quizzesInfo;
    fd_set readfds;
    fd_set masterfds;
    int server_fd;

} Context;

// Client list

void handle_new_client_connection(Context *context);
void handle_client(Client *client, Context *context);
void init_clients_info(ClientsInfo *clientsInfo);
void deallocate_clients(ClientsInfo *clientsInfo);

// Quiz

int load_quizzes_from_directory(const char *directory_path, QuizzesInfo *quizzesInfo);
void deallocate_quizzes(QuizzesInfo *quizzesInfo);

// Dashboard

void show_dashboard(Context *context);
void enable_raw_mode();
void disable_raw_mode();

// Ranking

RankingNode *create_ranking_node(Client *client);
void insert_ranking_node(Quiz *quiz, RankingNode *node);
void list_rankings(Quiz *quiz);
void list_completed_rankings(Quiz *quiz);
void update_ranking(RankingNode *node, Quiz *quiz);
void remove_ranking(RankingNode *node, Quiz *quiz);
void deallocate_ranking(RankingNode *head);

#endif // UTILS_H