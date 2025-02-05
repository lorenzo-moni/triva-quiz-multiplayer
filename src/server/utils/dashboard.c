#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include "utils.h"

/**
 * @brief Mostra i nomi dei quiz disponibili a schermo
 *
 * @param quizzesInfo puntatore alla struttura che contiene le informazioni sui quiz
 */
void show_quiz_names(QuizzesInfo *quizzesInfo)
{
  printf("Temi:\n");
  for (uint32_t i = 0; i < quizzesInfo->total_quizzes; ++i)
    printf("%d - %s\n", i + 1, quizzesInfo->quizzes[i]->name);
}
/**
 * @brief Mostra il numero dei client connessi e i rispettivi nickname
 *
 * @param clientsInfo puntatore alla struttura con le informazioni sui clients
 */
void show_clients(ClientsInfo *clientsInfo)
{
  printf("\nPartecipanti (%d)\n", clientsInfo->connected_clients);
  Client *current_client = clientsInfo->clients_head;
  while (current_client)
  {
    if (current_client->nickname)
      printf("- %s\n", current_client->nickname);
    current_client = current_client->next_node;
  }
}
/**
 * @brief Mostra la classifica di tutti i quiz
 *
 * @param quizzesInfo puntatore alla struttura che contiene le informazioni sui quiz
 */
void show_scores(QuizzesInfo *quizzesInfo)
{
  for (uint32_t i = 0; i < quizzesInfo->total_quizzes; i++)
  {
    printf("\nPunteggio Tema %d\n", i + 1);
    list_rankings(quizzesInfo->quizzes[i]);
  }
}
/**
 * @brief Mostra per ogni quiz il nickname dei clients che l'hanno completato
 *
 * @param quizzesInfo puntatore alla struttura che contiene le informazioni sui quiz
 */
void show_completed_quizes(QuizzesInfo *quizzesInfo)
{
  for (uint32_t i = 0; i < quizzesInfo->total_quizzes; i++)
  {
    printf("\nQuiz Tema %d completato\n", i + 1);
    list_completed_rankings(quizzesInfo->quizzes[i]);
  }
}

/**
 * @brief Stampa a schermo la dashboard del server
 *
 * @param context puntatore alla struttura che contiene le informazioni sul contesto del servizio
 */
void show_dashboard(Context *context)
{
  // pulisco lo schermo e svuoto il buffer di output
  system("clear");
  fflush(stdout);

  printf("Trivia Quiz\n");
  printf("+++++++++++++++++++++++++++\n");
  show_quiz_names(&context->quizzesInfo);
  printf("+++++++++++++++++++++++++++\n");
  show_clients(&context->clientsInfo);
  show_scores(&context->quizzesInfo);
  show_completed_quizes(&context->quizzesInfo);

  printf("\nDigitare 'q' per terminare il server: \n");
}
