# Compilatorul și flag-urile
CC = gcc
CFLAGS = -Wall -g

# Fisiere sursă
SRC = client.c helpers.c requests.c parson.c buffer.c
OBJ = $(SRC:.c=.o)

# Executabil
EXEC = client

# Regulă implicită
all: $(EXEC)

# Linkare finală
$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Curățare fișiere intermediare
clean:
	rm -f *.o $(EXEC)
