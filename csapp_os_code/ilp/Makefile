CFLAGS := -Wall -g -O2

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=)

# Default target
run: $(OBJS)
	perf stat -e cycles,instructions ./$(OBJS)

%: %.c
	gcc $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJS)

.PHONY: run clean
