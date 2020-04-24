CC=gcc
CFLAGS=-Wall 
EXENAME=testpl

SRC-DIR=pipeline
BUILD-DIR=build

SRCS=$(wildcard $(SRC-DIR)/*.c)
OBJS=$(patsubst $(SRC-DIR)%.c, $(BUILD-DIR)%.o, $(SRCS))

all: $(EXENAME)

$(BUILD-DIR)/%.o: $(SRC-DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) -c -o $@ $^ $(CFLAGS)

$(EXENAME): $(OBJS)
	$(CC) -o $@ $^

clean:
	rm -rf $(EXENAME) $(BUILD-DIR)/
