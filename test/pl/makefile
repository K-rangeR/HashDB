CC=gcc
CFLAGS=-Wall 
EXENAME=testpl

DB-DIR=../../src
SRC-DIR=pipeline
BUILD-DIR=build

PL-SRCS=$(wildcard $(SRC-DIR)/*.c)
OBJS=$(patsubst $(SRC-DIR)%.c, $(BUILD-DIR)%.o, $(PL-SRCS))

DB-SRCS=$(wildcard $(DB-DIR)/*.c)
OBJS+=$(patsubst $(DB-DIR)%.c, $(BUILD-DIR)%.o, $(DB-SRCS))

all: $(EXENAME)

$(BUILD-DIR)/%.o: $(SRC-DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) -c -o $@ $^ $(CFLAGS)

$(BUILD-DIR)/%.o: $(DB-DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) -c -o $@ $^ $(CFLAGS)

$(EXENAME): $(OBJS)
	$(CC) -o $@ $^

clean:
	rm -rf $(EXENAME) $(BUILD-DIR)/
