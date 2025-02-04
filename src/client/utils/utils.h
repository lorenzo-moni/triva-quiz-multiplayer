#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "../../common/common.h"
#include "stdbool.h"

#define ENDQUIZ "endquiz"
#define SHOWSCORE "show score"

// Dashboard

void show_menu();
void show_quiz_list(char **quizzes, int total_quizzes);
void clear_input_buffer();
bool initial_menu();

// Connection

void handle_nickname_selection(int server_fd, Message *msg);
void handle_quiz_selection(int server_fd, Message *msg);
void request_available_quizzes(int server_fd);
void handle_rankings(Message *msg);
void handle_error(Message *msg);
void handle_message(Message *msg);
void handle_selected_quiz(Message *msg);
void handle_quiz_question(int server_fd, Message *msg);
int get_console_input(char *buffer, int buffer_size);

#endif // UTILS_H