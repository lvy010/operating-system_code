CFLAGS := -Wall -g -static
CC := musl-gcc

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=)

# Default target
all: $(OBJS)
	strace -o to_file.log ./hello > /dev/null
	strace -o to_tty.log ./hello
	vimdiff to_file.log to_tty.log

%: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJS) *.log

.PHONY: all clean
