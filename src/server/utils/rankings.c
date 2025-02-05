#include "utils.h"

/**
 * @brief Crea un nuovo nodo RankingNode relativo a un client
 *
 * Questa funzione inizializza un nodo di tipo RankingNode relativo ad un client passato per parametro
 *
 * @param client puntatore al client a cui il RankingNode è relativo
 * @return nodo di tipo RankingNode inizializzato
 */
RankingNode *create_ranking_node(Client *client)
{
    RankingNode *new_node = malloc(sizeof(RankingNode));
    handle_malloc_error(new_node, "Errore nell'allocazione del RankingNode");
    new_node->client = client;
    new_node->is_quiz_completed = false;
    new_node->score = 0;
    new_node->current_question = 0;
    new_node->next_node = NULL;
    new_node->prev_node = NULL;

    return new_node;
}

/**
 * @brief Inserisce il RankingNode in coda alla classifica del Quiz
 *
 * Questa funzione inserisce un nodo RankingNode relativo ad un client per un quiz in coda alla classifica relativa a tale quiz
 *
 * @param quiz puntatore al quiz in coda alla quale classifica inserire il nodo
 * @param node puntatore al nodo da inserire in coda alla classifica del quiz
 */
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

/**
 * @brief Stampa a schermo la classifica relativa ad un quiz
 *
 * @param quiz puntatore al quiz di cui stampare la classifica a schermo
 */
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
/**
 * @brief Stampa a schermo il nickname degli utenti che hanno completato il quiz
 *
 * @param quiz puntatore al quiz di cui stampare gli utenti che lo hanno completato
 */
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

/**
 * @brief Aggiorna la classifica di un quiz
 *
 * Questa funzione sposta un nodo verso la testa della lista doppiamente concatenata relativa alla classifica di un quiz fino ad inserirlo
 * nella posizione corretta relativa al suo score
 *
 * @param node puntatore al nodo da spostare all'interno della classifica in base allo score
 * @param quiz puntatore al quiz di cui modificare la classifica
 */
void update_ranking(RankingNode *node, Quiz *quiz)
{
    if (node == NULL || quiz->ranking_head == NULL)
        return;

    // se il nodo è già in posizione corretta o in testa allora non faccio niente
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

/**
 * @brief Rimuove e dealloca un elemento della classifica di un quiz
 *
 * Questa funzione si occupa di rimuovere e deallocare un RankingNode dalla lista doppiamente concatenata relativa a un Quiz
 *
 * @param node puntatore al nodo da rimuovere e deallocare
 * @param quiz puntatore al quiz nella cui classifica si trova il nodo
 */
void remove_ranking(RankingNode *node, Quiz *quiz)
{
    if (node == NULL)
        return;
    if (quiz->ranking_head == NULL)
        return;

    quiz->total_clients -= 1;

    if (quiz->ranking_head == node)
        quiz->ranking_head = node->next_node;

    if (quiz->ranking_tail == node)
        quiz->ranking_tail = node->prev_node;

    if (node->prev_node != NULL)
        node->prev_node->next_node = node->next_node;

    if (node->next_node != NULL)
        node->next_node->prev_node = node->prev_node;

    free(node);
}

/**
 * @brief Rimuove e dealloca tutti i RankingNode di un quiz
 *
 * @param quiz puntatore al quiz di cui rimuovere la classifica
 */
void deallocate_rankings(Quiz *quiz)
{
    RankingNode *current = quiz->ranking_head;
    RankingNode *next;
    while (current)
    {
        next = current->next_node;
        free(current);
        current = next;
    }
    quiz->ranking_head = quiz->ranking_tail = NULL;
}