# Variabili configurabili
CC = gcc

INCDIRS = src/client src/server src/client/utils src/server/utils
CFLAGS = -Wall -Wextra $(addprefix -I, $(INCDIRS))

# Directories
SRC_DIR_CLIENT = src/client
SRC_DIR_SERVER = src/server
BUILD_DIR_CLIENT = build/client
BUILD_DIR_SERVER = build/server
BUILD_DIR = build

# Output files
CLIENT_EXEC = client
SERVER_EXEC = server

# Source and object files for client
CLIENT_SRC = $(shell find $(SRC_DIR_CLIENT) -name '*.c')
CLIENT_OBJ = $(CLIENT_SRC:$(SRC_DIR_CLIENT)/%.c=$(BUILD_DIR_CLIENT)/%.o)

# Source and object files for server
SERVER_SRC = $(shell find $(SRC_DIR_SERVER) -name '*.c')
SERVER_OBJ = $(SERVER_SRC:$(SRC_DIR_SERVER)/%.c=$(BUILD_DIR_SERVER)/%.o)

# Default target
all: $(CLIENT_EXEC) $(SERVER_EXEC)

# Rule to build the client executable
$(CLIENT_EXEC): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) $(CLIENT_OBJ) -o $@

# Rule to build the server executable
$(SERVER_EXEC): $(SERVER_OBJ)
	$(CC) $(CFLAGS) $(SERVER_OBJ) -o $@

# Rule to compile client object files
$(BUILD_DIR_CLIENT)/%.o: $(SRC_DIR_CLIENT)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to compile server object files
$(BUILD_DIR_SERVER)/%.o: $(SRC_DIR_SERVER)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

# Phony targets
.PHONY: all clean
