#include "utils.h"

// La funzione create_ranking_node inizializza un nodo della classifica
RankingNode *create_ranking_node(Client *client)
{
    RankingNode *new_node = malloc(sizeof(RankingNode));
    new_node->client = client;
    new_node->is_quiz_completed = 0;
    new_node->score = 0;
    new_node->current_question = 0;
    new_node->next_node = NULL;
    new_node->prev_node = NULL;

    return new_node;
}

// La funzione insert_ranking_node inserisce un nodo RankingNode relativo ad un client per un quiz in coda alla classifica relativa al quiz
void insert_ranking_node(Quiz *quiz, RankingNode *node)
{
    if (!node)
        return;

    if (!quiz->ranking_head)
    {
        quiz->ranking_head = quiz->ranking_tail = node;
        return;
    }
    quiz->ranking_tail->next_node = node;
    node->prev_node = quiz->ranking_tail;
    quiz->ranking_tail = node;
}

// La funzione list_rankings stampa a schermo il ranking relativo ad un determinato quiz
void list_rankings(Quiz *quiz)
{
    RankingNode *current = quiz->ranking_head;
    if (!current)
        printf("------\n");
    while (current)
    {
        printf("- %s %d\n", current->client->nickname, current->score);
        current = current->next_node;
    }
}

// La funzione list_completed_rankings stampa a schermo il nickname degli utenti che hanno completato il quiz
void list_completed_rankings(Quiz *quiz)
{
    RankingNode *current = quiz->ranking_head;
    int counter = 0;
    while (current)
    {
        if (current->is_quiz_completed)
        {
            printf("- %s\n", current->client->nickname);
            counter += 1;
        }
        current = current->next_node;
    }
    if (!counter)
        printf("------\n");
}

// La funzione update_ranking sposta un nodo verso la testa della lista doppiamente concatenata relativa alla classifica di un quiz
void update_ranking(RankingNode *node, Quiz *quiz)
{
    if (node == NULL || quiz->ranking_head == NULL)
        return;

    // Se il nodo è già in posizione corretta o in testa allora non faccio niente
    if (node->prev_node == NULL || node->score <= node->prev_node->score)
        return;

    // vado a rimuovere il nodo dalla posizione corrente
    if (node->next_node != NULL)
        node->next_node->prev_node = node->prev_node;
    if (node->prev_node != NULL)
        node->prev_node->next_node = node->next_node;

    // trovo la posizione corretta
    RankingNode *current = node->prev_node;
    while (current != NULL && current->score < node->score)
        current = current->prev_node;

    // inserisco il nodo nella nuova posizione
    if (current == NULL)
    {
        // inserisco il nodo in testa
        node->next_node = quiz->ranking_head;
        node->prev_node = NULL;
        quiz->ranking_head->prev_node = node;
        quiz->ranking_head = node;
    }
    else
    {
        // inserisco il nodo dopo di current
        node->next_node = current->next_node;
        node->prev_node = current;
        if (current->next_node != NULL)
        {
            current->next_node->prev_node = node;
        }
        current->next_node = node;
    }
}

// La funzione remove_ranking si occupa di rimuovere e di deallocare un nodo node da una lista doppiamente concatenata relativa al quiz
void remove_ranking(RankingNode *node, Quiz *quiz)
{
    if (node == NULL)
        return;
    if (quiz->ranking_head == NULL)
        return;

    if (quiz->ranking_head == node)
        quiz->ranking_head = node->next_node;

    if (node->prev_node != NULL)
        node->prev_node->next_node = node->next_node;

    if (node->next_node != NULL)
        node->next_node->prev_node = node->prev_node;

    free(node);
}