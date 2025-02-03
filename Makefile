# Variabili configurabili
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -fanalyzer -Wstrict-overflow=5 -Wundef -Winline -Wconversion -Wshadow -Wpointer-arith -Wcast-qual -I$(SRC_DIR)/common -I$(SRC_DIR)/client/utils -I$(SRC_DIR)/server/utils


# Directory
SRC_DIR = src
BUILD_DIR = build
CLIENT_BUILD_DIR = $(BUILD_DIR)/client
SERVER_BUILD_DIR = $(BUILD_DIR)/server


# Eseguibili
CLIENT_EXEC = client
SERVER_EXEC = server

# Sorgenti e oggetti per il client
CLIENT_SRC = $(SRC_DIR)/client/client.c \
             $(SRC_DIR)/client/utils/connection.c \
             $(SRC_DIR)/client/utils/dashboard.c \
             $(SRC_DIR)/common/common.c

CLIENT_OBJ = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(CLIENT_SRC))

# Sorgenti e oggetti per il server
SERVER_SRC = $(SRC_DIR)/server/server.c \
             $(SRC_DIR)/server/utils/dashboard.c \
             $(SRC_DIR)/server/utils/handle_clients.c \
			 $(SRC_DIR)/server/utils/quizzes.c \
			 $(SRC_DIR)/server/utils/rankings.c \
			 $(SRC_DIR)/common/common.c

SERVER_OBJ = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SERVER_SRC))


# Target di default
all: $(CLIENT_EXEC) $(SERVER_EXEC)

# Regola per compilare l'eseguibile del client
$(CLIENT_EXEC): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) $(CLIENT_OBJ) -o $@

# Regola per compilare l'eseguibile del server
$(SERVER_EXEC): $(SERVER_OBJ)
	$(CC) $(CFLAGS) $(SERVER_OBJ) -o $@

# Regola generica per compilare i file oggetto
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Pulizia
clean:
	rm -rf $(BUILD_DIR) $(CLIENT_EXEC) $(SERVER_EXEC)

# Target fittizi
.PHONY: all clean client server