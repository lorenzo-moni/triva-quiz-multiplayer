#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "../../common/common.h"

/**
 * @brief Indicates the state of a given Client
 *
 * This structure stores the current state of the client.
 */
typedef enum ClientState
{
    LOGIN,          /**< The client is logging in. */
    LOGGED_IN,      /**< The client has successfully logged in. */
    SELECTING_QUIZ, /**< The client is selecting a quiz. */
    PLAYING         /**< The client is participating in a quiz. */
} ClientState;

/**
 * @brief Represents a client connected to the server
 *
 * This structure contains information related to a client,
 * including the connection socket, the nickname, the current state,
 * the quiz scores, and pointers for managing a linked list.
 */
typedef struct Client
{
    int socket_fd;                        /**< File descriptor of the client's socket. */
    char *nickname;                       /**< Client's nickname. */
    ClientState state;                    /**< Current state of the client. */
    struct RankingNode **client_rankings; /**< Array of the client's rankings in each available quiz. */
    unsigned int current_quiz_id;         /**< ID of the quiz in which the client is participating. (-1 if not participating in any quiz) */
    struct Client *prev_node;             /**< Pointer to the previous client in the client list. */
    struct Client *next_node;             /**< Pointer to the next client in the list. */
} Client;

/**
 * @brief Contains information on all connected clients
 *
 * This structure manages the list of currently connected clients and contains
 * pointers to the head and tail of the doubly linked list that holds
 * all clients, the number of currently connected clients, and the maximum file descriptor
 * among the connected clients.
 */
typedef struct ClientsInfo
{
    struct Client *clients_head;    /**< Pointer to the first client in the list. */
    struct Client *clients_tail;    /**< Pointer to the last client in the list. */
    unsigned int connected_clients; /**< Total number of currently connected clients. */
    int max_fd;                     /**< Maximum file descriptor value among the clients. */
} ClientsInfo;

/**
 * @brief Contains information related to a quiz question
 *
 * This structure contains information regarding a quiz question,
 * including the question text, the total number of answers, and the text of the answers.
 */
typedef struct QuizQuestion
{
    char *question;    /**< Text of the quiz question. */
    char **answers;    /**< Array of strings containing the possible answers. */
    int total_answers; /**< Total number of possible answers. */
} QuizQuestion;

/**
 * @brief Contains information related to a quiz
 *
 * This structure contains information regarding a quiz,
 * including the name, the questions, and pointers to the head and tail of the doubly linked list
 * representing the quiz ranking.
 */
typedef struct Quiz
{
    char *name;                       /**< Name of the quiz. */
    QuizQuestion **questions;         /**< Array of pointers to the quiz questions. */
    uint16_t total_questions;         /**< Total number of questions in the quiz. */
    uint16_t total_clients;           /**< Number of clients in the ranking. */
    struct RankingNode *ranking_head; /**< Pointer to the head of the ranking list. */
    struct RankingNode *ranking_tail; /**< Pointer to the tail of the ranking list. */
} Quiz;

/**
 * @brief Contains information on all available quizzes
 *
 * This structure manages an array of pointers to the available quizzes
 * and the total number of quizzes.
 */
typedef struct QuizzesInfo
{
    Quiz **quizzes;         /**< Array of pointers to the available quizzes. */
    uint16_t total_quizzes; /**< Total number of available quizzes. */
} QuizzesInfo;

/**
 * @brief Node of the ranking list for a quiz
 *
 * This structure represents a node in the doubly linked ranking list of quiz participants,
 * containing information about the client, the score, the quiz completion status,
 * and pointers to the previous and next nodes in the list.
 */
typedef struct RankingNode
{
    Client *client;                /**< Pointer to the client associated with this node. */
    uint16_t score;                /**< Score obtained by the client in the quiz. */
    bool is_quiz_completed;        /**< Indicates if the client has completed the quiz. */
    unsigned int current_question; /**< ID of the question the client needs to answer. */
    struct RankingNode *prev_node; /**< Pointer to the previous node in the ranking list. */
    struct RankingNode *next_node; /**< Pointer to the next node in the ranking list. */
} RankingNode;

/**
 * @brief Global context of the server application
 *
 * This structure groups all information related to the server's state and allows
 * for easy transfer of information through function calls.
 */
typedef struct Context
{
    ClientsInfo clientsInfo; /**< Information about connected clients. */
    QuizzesInfo quizzesInfo; /**< Information about available quizzes. */
    fd_set readfds;          /**< Set of file descriptors managed by select with sockets ready for reading. */
    fd_set masterfds;        /**< Master set of file descriptors. */
    int server_fd;           /**< File descriptor of the server's listener socket. */
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
void deallocate_rankings(Quiz *quiz);

#endif // SERVER_UTILS_H
