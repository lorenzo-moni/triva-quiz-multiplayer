#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <limits.h>
#include <sys/select.h>
#include <string.h>

// Client list

typedef struct ClientNode
{
    int socket_fd;
    char *nickname;
    struct ClientNode *next_client;

} ClientNode;

ClientNode *create_node(int socket_fd);
void add_client(ClientNode **head, int socket_fd);
void remove_client(ClientNode **head, int socket_fd);
void iterate_clients(ClientNode *head, fd_set *readfds, int *max_sd);

// Ranking List

typedef struct RankingNode
{
    ClientNode *client;
    int score;
    int is_quiz_completed;
    struct RankingNode *prev_node;
    struct RankingNode *next_node;

} RankingNode;

// Quiz

typedef struct Quiz
{
    char *name;
    char **questions;
    char **answers;
    int total_questions;
    RankingNode *ranking_head;
    RankingNode *ranking_tail;
} Quiz;

Quiz *load_quiz_from_file(const char *file_path);
Quiz **load_quizzes_from_directory(const char *directory_path, int *total_quizzes);

#endif // UTILS_H