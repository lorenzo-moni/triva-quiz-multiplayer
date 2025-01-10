#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

void show_quiz_names(QuizzesInfo *quizzesInfo)
{
  printf("Temi:\n");
  for (int i = 0; i < quizzesInfo->total_quizzes; ++i)
    printf("%d - %s\n", i + 1, quizzesInfo->quizzes[i]->name);
}

void show_clients(ClientsInfo *clientsInfo)
{
  printf("\nPartecipanti (%d)\n", clientsInfo->connected_clients);
  for (int i = 0; i <= clientsInfo->max_fd; i++)
    if (clientsInfo->clients[i].socket_fd != -1 && clientsInfo->clients[i].nickname)
      printf("- %s\n", clientsInfo->clients[i].nickname);
}

void show_scores(QuizzesInfo *quizzesInfo)
{
  for (int i = 0; i < quizzesInfo->total_quizzes; i++)
  {
    printf("\nPunteggio Tema %d\n", i + 1);
    list_rankings(quizzesInfo->quizzes[i]);
  }
}

void show_completed_quizes(QuizzesInfo *quizzesInfo)
{
  for (int i = 0; i < quizzesInfo->total_quizzes; i++)
  {
    printf("\nQuiz Tema %d completato\n", i + 1);
    list_completed_rankings(quizzesInfo->quizzes[i]);
  }
}

void show_dashboard(QuizzesInfo *quizzesInfo, ClientsInfo *clientsInfo, int score)
{
  printf("Trivia Quiz\n");
  printf("+++++++++++++++++++++++++++\n");
  show_quiz_names(quizzesInfo);
  printf("+++++++++++++++++++++++++++\n");
  if (!score)
    return;
  show_clients(clientsInfo);
  show_scores(quizzesInfo);
  show_completed_quizes(quizzesInfo);
}
