# directory
SRC_DIR = src
BUILD_DIR = build
CLIENT_BUILD_DIR = $(BUILD_DIR)/client
SERVER_BUILD_DIR = $(BUILD_DIR)/server

# configurable variables
CC = gcc
CFLAGS = -Wall -I$(SRC_DIR)/common -I$(SRC_DIR)/client/utils -I$(SRC_DIR)/server/utils

# executables
CLIENT_EXEC = client
SERVER_EXEC = server

# sources and objects for the client
CLIENT_SRC = $(SRC_DIR)/client/client.c \
             $(SRC_DIR)/client/utils/connection.c \
             $(SRC_DIR)/client/utils/dashboard.c \
             $(SRC_DIR)/common/common.c

CLIENT_OBJ = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(CLIENT_SRC))

# sources and objects for the server
SERVER_SRC = $(SRC_DIR)/server/server.c \
             $(SRC_DIR)/server/utils/dashboard.c \
             $(SRC_DIR)/server/utils/clients.c \
			 $(SRC_DIR)/server/utils/quizzes.c \
			 $(SRC_DIR)/server/utils/rankings.c \
			 $(SRC_DIR)/common/common.c

SERVER_OBJ = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SERVER_SRC))

# default target
all: $(CLIENT_EXEC) $(SERVER_EXEC)

# rule to compile the client executable
$(CLIENT_EXEC): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) $(CLIENT_OBJ) -o $@

# rule to compile the server executable
$(SERVER_EXEC): $(SERVER_OBJ)
	$(CC) $(CFLAGS) $(SERVER_OBJ) -o $@

# generic rule to compile object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# rule to remove the build directory and executables
clean:
	rm -rf $(BUILD_DIR) $(CLIENT_EXEC) $(SERVER_EXEC)

.PHONY: all clean client server
