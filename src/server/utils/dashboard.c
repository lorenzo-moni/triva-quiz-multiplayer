#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

/**
 * @brief Displays the names of the available quizzes on screen
 *
 * @param quizzesInfo pointer to the structure containing quiz information
 */
void show_quiz_names(QuizzesInfo *quizzesInfo)
{
  printf("Quizzes:\n");
  for (uint16_t i = 0; i < quizzesInfo->total_quizzes; ++i)
    printf("%d - %s\n", i + 1, quizzesInfo->quizzes[i]->name);
}

/**
 * @brief Displays the number of connected clients and their nicknames
 *
 * @param clientsInfo pointer to the structure containing client information
 */
void show_clients(ClientsInfo *clientsInfo)
{
  printf("\nParticipants (%d)\n", clientsInfo->connected_clients);
  Client *current_client = clientsInfo->clients_head;
  while (current_client)
  {
    if (current_client->nickname)
      printf("- %s\n", current_client->nickname);
    current_client = current_client->next_node;
  }
}

/**
 * @brief Displays the ranking of all quizzes
 *
 * @param quizzesInfo pointer to the structure containing quiz information
 */
void show_scores(QuizzesInfo *quizzesInfo)
{
  for (uint16_t i = 0; i < quizzesInfo->total_quizzes; i++)
  {
    printf("\nScore for Quiz %d\n", i + 1);
    list_rankings(quizzesInfo->quizzes[i]);
  }
}

/**
 * @brief Displays the nicknames of clients who completed each quiz
 *
 * @param quizzesInfo pointer to the structure containing quiz information
 */
void show_completed_quizes(QuizzesInfo *quizzesInfo)
{
  for (uint16_t i = 0; i < quizzesInfo->total_quizzes; i++)
  {
    printf("\nQuiz %d completed\n", i + 1);
    list_completed_rankings(quizzesInfo->quizzes[i]);
  }
}

/**
 * @brief Prints the server dashboard on screen
 *
 * @param context pointer to the structure containing the service context information
 */
void show_dashboard(Context *context)
{
  // Clear the screen and flush the output buffer
  system("clear");
  fflush(stdout);

  printf("Trivia Quiz\n");
  printf("+++++++++++++++++++++++++++\n");
  show_quiz_names(&context->quizzesInfo);
  printf("+++++++++++++++++++++++++++\n");
  show_clients(&context->clientsInfo);
  show_scores(&context->quizzesInfo);
  show_completed_quizes(&context->quizzesInfo);

  printf("\nType 'q' to terminate the server: \n");
}
