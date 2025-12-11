CFLAGS := -Wall -g -O2
CFLAGS += -z execstack -fno-stack-protector -fno-pie

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=)

# Default target
all: $(OBJS)

%: %.c
	musl-gcc $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJS)

.PHONY: all clean
