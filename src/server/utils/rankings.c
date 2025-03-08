#include "utils.h"

/**
 * @brief Creates a new RankingNode for a client
 *
 * This function initializes a RankingNode for a client passed as a parameter.
 *
 * @param client pointer to the client associated with the RankingNode
 * @return initialized RankingNode
 */
RankingNode *create_ranking_node(Client *client)
{
    RankingNode *new_node = malloc(sizeof(RankingNode));
    handle_malloc_error(new_node, "Error allocating RankingNode");
    new_node->client = client;
    new_node->is_quiz_completed = false;
    new_node->score = 0;
    new_node->current_question = 0;
    new_node->next_node = NULL;
    new_node->prev_node = NULL;

    return new_node;
}

/**
 * @brief Inserts the RankingNode at the end of the Quiz ranking list
 *
 * This function inserts a RankingNode for a client at the tail of the ranking list for a quiz.
 *
 * @param quiz pointer to the quiz whose ranking list will have the node inserted
 * @param node pointer to the node to be inserted at the end of the quiz ranking list
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
 * @brief Displays the ranking list for a quiz on screen
 *
 * @param quiz pointer to the quiz for which to display the ranking list
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
 * @brief Displays the nicknames of users who have completed the quiz on screen
 *
 * @param quiz pointer to the quiz for which to display the users that have completed it
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
 * @brief Updates the ranking for a quiz
 *
 * This function moves a node towards the head of the doubly linked ranking list for a quiz until it is placed
 * in the correct position based on its score.
 *
 * @param node pointer to the node to reposition within the ranking based on its score
 * @param quiz pointer to the quiz whose ranking is to be updated
 */
void update_ranking(RankingNode *node, Quiz *quiz)
{
    if (node == NULL || quiz->ranking_head == NULL)
        return;

    // If the node is already in the correct position or at the head, do nothing
    if (node->prev_node == NULL || node->score <= node->prev_node->score)
        return;

    // Remove the node from its current position
    if (node->next_node != NULL)
        node->next_node->prev_node = node->prev_node;
    if (node->prev_node != NULL)
        node->prev_node->next_node = node->next_node;

    // Find the correct position
    RankingNode *current = node->prev_node;
    while (current != NULL && current->score < node->score)
        current = current->prev_node;

    // Insert the node in the new position
    if (current == NULL)
    {
        // Insert the node at the head
        node->next_node = quiz->ranking_head;
        node->prev_node = NULL;
        quiz->ranking_head->prev_node = node;
        quiz->ranking_head = node;
    }
    else
    {
        // Insert the node after current
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
 * @brief Removes and deallocates an element from a quiz ranking
 *
 * This function removes and deallocates a RankingNode from the doubly linked ranking list of a quiz.
 *
 * @param node pointer to the node to be removed and deallocated
 * @param quiz pointer to the quiz whose ranking contains the node
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
 * @brief Removes and deallocates all RankingNodes of a quiz
 *
 * @param quiz pointer to the quiz whose ranking list is to be deallocated
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
