# Creates a shared library for the hashDB code
CC=gcc
CFLAGS=-Wall 
LIB=hashDB.so

SRC=src
BUILD-DIR=build

SRCS=$(wildcard $(SRC)/*.c)
OBJS=$(patsubst $(SRC)%.c, $(BUILD-DIR)%.o, $(SRCS))

all: $(LIB)

$(BUILD-DIR)/%.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) -c -o $@ $^ $(CFLAGS)

$(LIB): $(OBJS)
	$(CC) -shared -o $(LIB) $^

clean:
	rm -rf $(LIB) $(BUILD-DIR)/
