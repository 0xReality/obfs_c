
CC = gcc

CFLAGS = -Wall -Wextra -std=c99 -pedantic

TARGET = obfs

SRCS = main.c obfuscator.c utils.c hashtable.c

HEADERS = utils.h obfuscator.h hashtable.h

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
.PHONY: all clean run
