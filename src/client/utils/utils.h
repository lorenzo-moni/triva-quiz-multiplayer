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

#define PORT 8080
#define BUFFER_SIZE 1024

void show_menu();
void show_quiz_list(char **quizzes, int total_quizzes);

void handle_nickname_selection(int server_fd);
// void handle_quiz_selection(int server_fd, Message *msg, int *stop);
// void request_available_quizzes(int server_fd);
// void deserialize_quiz_list(Message *msg, char ***quizzes, int *total_quizzes);
// void handle_error(Message *msg);
// void handle_message(Message *msg);
// void handle_quiz_question(int server_fd, Message *msg, int *stop);
void get_console_input(char *buffer, int buffer_size, int *stop);
void clear_input_buffer();
bool initial_menu();

#endif // UTILS_H