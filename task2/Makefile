CC = gcc
CFLAGS = -Wall -Werror

CLIENT_SRC = task2_sender.c

CLIENT_TARGET = client
SERVER_TARGET = server

$(CLIENT_TARGET): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $(CLIENT_TARGET) $(CLIENT_SRC)

clean:
	rm -f $(CLIENT_TARGET)