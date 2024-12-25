# Variabili configurabili
CC = gcc
CFLAGS = -Wall -Wextra $(addprefix -I, $(INCDIRS))
BINARY = server
CODEDIR = src
INCDIRS = src src/utils
OBJDIR = build

# Trova tutti i file .c nella directory src (ricorsivamente)
SRC_FILES = $(shell find $(CODEDIR) -name '*.c')

# Genera i file .o corrispondenti in build
OBJ_FILES = $(patsubst $(CODEDIR)/%.c, $(OBJDIR)/%.o, $(SRC_FILES))

# Regola di default
all: $(BINARY)

# Regola per creare l'eseguibile
$(BINARY): $(OBJ_FILES)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $^

# Regola per compilare i file .o
$(OBJDIR)/%.o: $(CODEDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Pulizia
clean:
	rm -rf $(OBJDIR)

# Fornisce informazioni utili all'utente
.PHONY: all clean
