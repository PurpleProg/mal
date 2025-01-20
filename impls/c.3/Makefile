CFLAGS = -Wall -g
SRCS = step1_read_print.c reader.c linked_list.c printer.c
OBJS = $(SRCS:.c=.o)

.PHONNY: all clean

all: step1_read_print

clean:
	rm -f $(OBJS)


step0_repl: step0_repl.c
	gcc -Wall step0_repl.c -o step0_repl

step1_read_print: $(OBJS)
	gcc $(CFLAGS) $(OBJS) -o $@ -lpcre -lgc

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@
