#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
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
  Client *current_client = clientsInfo->clients_head;
  while (current_client)
  {
    if (current_client->nickname)
      printf("- %s\n", current_client->nickname);
    current_client = current_client->next_node;
  }
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

void enable_raw_mode()
{
  struct termios term;
  tcgetattr(STDIN_FILENO, &term);          // Ottieni le impostazioni correnti
  term.c_lflag &= ~(ICANON | ECHO);        // Disabilita modalità canonica ed echo
  tcsetattr(STDIN_FILENO, TCSANOW, &term); // Applica le modifiche
}

void disable_raw_mode()
{
  struct termios term;
  tcgetattr(STDIN_FILENO, &term);          // Ottieni le impostazioni correnti
  term.c_lflag |= (ICANON | ECHO);         // Ripristina modalità canonica ed echo
  tcsetattr(STDIN_FILENO, TCSANOW, &term); // Applica le modifiche
}

void show_dashboard(Context *context)
{

  // system("clear"); // Pulisco lo schermo
  // fflush(stdout);  // Svuoto il buffer di output

  printf("Trivia Quiz\n");
  printf("+++++++++++++++++++++++++++\n");
  show_quiz_names(&context->quizzesInfo);
  printf("+++++++++++++++++++++++++++\n");
  show_clients(&context->clientsInfo);
  show_scores(&context->quizzesInfo);
  show_completed_quizes(&context->quizzesInfo);

  printf("\nPremere 'q' per terminare il server: \n");
}
