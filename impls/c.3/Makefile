CFLAGS = -Wall -g -Wextra -pedantic -O1
SRCS = step4_if_fn_do.c core.c reader.c linked_list.c printer.c hashmap.c env.c
# SRCS = $(wildcard *.c)
# EXCLUDED_SRCS = step0_repl.c, step1_read_print.c, step2_eval.c, step3_env.c
# SRCS = $(filter-out $(EXCLUDED_SRCS), $(ALL_SRCS))
OBJS = $(SRCS:.c=.o)

.PHONNY: all clean

all: step4_if_fn_do

clean:
	rm -f $(OBJS)


step0_repl: step0_repl.c
	gcc -Wall step0_repl.c -o step0_repl

step1_read_print: $(OBJS)
	gcc $(CFLAGS) $(OBJS) -o $@ -lpcre -lgc

step2_eval: $(OBJS)
	gcc $(CFLAGS) $(OBJS) -o $@ -lpcre -lgc

step3_env: $(OBJS)
	gcc $(CFLAGS) $(OBJS) -o $@ -lpcre -lgc

step4_if_fn_do: $(OBJS)
	gcc $(CFLAGS) $(OBJS) -o $@ -lpcre -lgc

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@
