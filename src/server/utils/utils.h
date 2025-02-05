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
 * @brief Indica lo stato in cui si trova un determinato Client
 *
 * Questa struttura memorizza lo stato corrente del client
 */
typedef enum ClientState
{
    LOGIN,          /**< Il client sta effettuando il login. */
    LOGGED_IN,      /**< Il client ha effettuato con successo il login. */
    SELECTING_QUIZ, /**< Il client sta selezionando un quiz. */
    PLAYING         /**< Il client sta partecipando a un quiz. */
} ClientState;

/**
 * @brief Rappresenta un client connesso al server
 *
 * Questa struttura contiene informazioni relative a un client,
 * inclusi il socket di connessione, il nickname, lo stato corrente,
 * i punteggi nei quiz e i puntatori per la gestione di una lista collegata.
 */
typedef struct Client
{
    int socket_fd;                        /**< File descriptor del socket del client. */
    char *nickname;                       /**< Nickname del client. */
    ClientState state;                    /**< Stato attuale del client. */
    struct RankingNode **client_rankings; /**< Array dei ranking del client in ogni quiz disponibile */
    unsigned int current_quiz_id;         /**< ID del quiz a cui il client sta partecipando. (-1 se non sta partecipando a nessun quiz) */
    struct Client *prev_node;             /**< Puntatore al precedente client nella lista dei clients. */
    struct Client *next_node;             /**< Puntatore al prossimo client nella lista. */
} Client;

/**
 * @brief Contiene informazioni su tutti i client connessi
 *
 * Questa struttura gestisce la lista dei client attualmente connessi e presenta al suo interno
 * il puntatore alla testa e alla coda della lista doppiamente concatenata che contiene
 * tutti i clients, il numero di clients connessi e il massimo file descript relativo ai client connessi
 */
typedef struct ClientsInfo
{
    struct Client *clients_head;    /**< Puntatore al primo client nella lista. */
    struct Client *clients_tail;    /**< Puntatore all'ultimo client nella lista. */
    unsigned int connected_clients; /**< Numero totale di client attualmente connessi. */
    int max_fd;                     /**< Valore massimo del file descriptor tra i client. */
} ClientsInfo;

/**
 * @brief Contiene informazioni relative ad una domanda di un quiz
 *
 * Questa struttura contiene al suo interno le informazioni riguardanti una domanda di un quiz
 * In particolare contiene il testo della domanda, il numero totale di risposte e il testo delle risposte *
 */
typedef struct QuizQuestion
{
    char *question;    /**< Testo della domanda del quiz. */
    char **answers;    /**< Array di stringhe contenente le possibili risposte. */
    int total_answers; /**< Numero totale di possibili. */
} QuizQuestion;

/**
 * @brief Contiene informazioni relative ad un quiz
 *
 * Questa struttura contiene al suo interno le informazioni riguardanti un quiz
 * In particolare contiene il nome, le domande e i puntatori alla coda e alla testa della lista doppiamente concatenata
 * relativa alla classifica del quiz
 */
typedef struct Quiz
{
    char *name;                       /**< Nome del quiz. */
    QuizQuestion **questions;         /**< Array di puntatori alle domande del quiz. */
    uint32_t total_questions;         /**< Numero totale di domande nel quiz. */
    uint32_t total_clients;           /**< Numero di client che sono in classifica */
    struct RankingNode *ranking_head; /**< Puntatore alla testa della lista della classifica. */
    struct RankingNode *ranking_tail; /**< Puntatore alla coda della lista della classifica. */
} Quiz;
/**
 * @brief Contiene informazioni su tutti i quiz disponibili
 *
 * Questa struttura gestisce un array di puntatori ai quiz disponibili
 * e il numero totale di quiz.
 */
typedef struct QuizzesInfo
{
    Quiz **quizzes;         /**< Array di puntatori ai quiz disponibili. */
    uint32_t total_quizzes; /**< Numero totale di quiz disponibili. */
} QuizzesInfo;

/**
 * @brief Nodo della lista di ranking per un quiz
 *
 * Questa struttura rappresenta un nodo nella lista doppiamente concatenata di ranking dei partecipanti
 * a un quiz, contenente informazioni sul client, il punteggio, lo stato di completamento
 * del quiz e i puntatori ai nodi precedente e successivo nella lista.
 */
typedef struct RankingNode
{
    Client *client;                /**< Puntatore al client associato a questo nodo. */
    uint32_t score;                /**< Punteggio ottenuto dal client nel quiz. */
    bool is_quiz_completed;        /**< Indica se il client ha completato il quiz */
    unsigned int current_question; /**< ID della domanda a cui il client deve rispondere. */
    struct RankingNode *prev_node; /**< Puntatore al nodo precedente nella lista di ranking. */
    struct RankingNode *next_node; /**< Puntatore al nodo successivo nella lista di ranking. */
} RankingNode;

/**
 * @brief Contesto globale dell'applicazione server
 *
 * Questa struttura raggruppa tutte le informazioni relative allo stato del server e permette
 * un agevole passaggio delle informazioni attraverso le chiamate di funzione *
 */
typedef struct Context
{
    ClientsInfo clientsInfo; /**< Informazioni sui client connessi. */
    QuizzesInfo quizzesInfo; /**< Informazioni sui quiz disponibili. */
    fd_set readfds;          /**< Set di file descriptor gestito dalla select con i socket pronti per la lettura. */
    fd_set masterfds;        /**< Set principale di file descriptor. */
    int server_fd;           /**< File descriptor del socket listener del server. */

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