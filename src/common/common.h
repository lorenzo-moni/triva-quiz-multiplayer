#ifndef COMMON_H
#define COMMON_H

#define ERROR_CODE "0"
#define SUCCESS_CODE "1"

#include "stddef.h"

#define MAX_PAYLOAD_SIZE 256

void receive_msg(int source_fd, char *payload);
void send_msg(int dest_fd, char *payload);

#endif