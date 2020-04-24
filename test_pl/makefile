CC=gcc
CFLAGS=-Wall 
EXENAME=testpl

SRCDIR=pipeline

SRCS=$(wildcard $(SRCDIR)/*.c)
OBJS=$(patsubst $(SRCDIR)%.c, build%.o, $(SRCS))

all: $(EXENAME)

build/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) -c -o $@ $^ $(CFLAGS)

$(EXENAME): $(OBJS)
	$(CC) -o $@ $^

clean:
	rm -rf $(EXENAME) build/
