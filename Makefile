CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

# Our source files (adjust if you add more .c files)
SRCS = src/parser.c src/lexer.c

# Objects are named after the .c files but with .o
OBJS = $(SRCS:.c=.o)

# The final binary we produce
TARGET = compiler

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
