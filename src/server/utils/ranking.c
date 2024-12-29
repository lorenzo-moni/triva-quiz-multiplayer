#include "utils.h"

RankingNode *create_ranking_node(Client *client, int selected_quiz_idx)
{
    RankingNode *new_node = malloc(sizeof(RankingNode));
    new_node->client = client;
    new_node->is_quiz_completed = 0;
    new_node->score = 0;
    new_node->next_node = NULL;
    new_node->prev_node = NULL;

    client->client_rankings[selected_quiz_idx] = new_node;
    return new_node;
}

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

void list_ranking(Quiz *quiz)
{
    RankingNode *current = quiz->ranking_head;
    while (current)
    {
        printf("%s %d\n", current->client->nickname, current->score);
        current = current->next_node;
    }
}