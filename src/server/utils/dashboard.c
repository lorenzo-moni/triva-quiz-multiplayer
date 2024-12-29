#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

void show_quiz_names(QuizzesInfo *quizzesInfo)
{
  for (int i = 0; i < quizzesInfo->total_quizzes; ++i)
    printf("%d - %s\n", i + 1, quizzesInfo->quizzes[i]->name);
}

void show_clients(ClientsInfo *clientsInfo)
{
  printf("Partecipanti (%d)\n", clientsInfo->connected_clients);
  for (int i = 0; i <= clientsInfo->max_fd; i++)
    if (clientsInfo->clients[i].state != WAITING_FOR_NICKNAME)
      printf("- %s\n", clientsInfo->clients[i].nickname);
}

void show_scores(QuizzesInfo *quizzesInfo)
{
  for (int i = 0; i < quizzesInfo->total_quizzes; i++)
  {
    if (quizzesInfo->quizzes[i]->ranking_head == NULL)
      continue;
    printf("\nPunteggio Tema %d\n", i + 1);
    list_ranking(quizzesInfo->quizzes[i]);
  }
}

void show_dashboard(QuizzesInfo *quizzesInfo, ClientsInfo *clientsInfo)
{
  printf("Trivia Quiz\n");
  printf("+++++++++++++++++++++++++++\n");
  show_quiz_names(quizzesInfo);
  printf("+++++++++++++++++++++++++++\n\n");
  show_clients(clientsInfo);
  show_scores(quizzesInfo);
}
