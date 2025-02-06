#ifndef CLIENT_UTILS_H
#define CLIENT_UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "../../common/common.h"
#include "stdbool.h"

// Dashboard

void show_menu();
void handle_selected_quiz(Message *msg);

// Connection

void handle_nickname_selection(int server_fd, Message *msg);
void handle_quiz_selection(int server_fd, Message *msg);
void request_available_quizzes(int server_fd);
void handle_rankings(Message *msg);
void handle_message(Message *msg);
void handle_quiz_question(int server_fd, Message *msg);

#endif // CLIENT_UTILS_H