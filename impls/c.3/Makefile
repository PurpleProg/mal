CFLAGS = -Wall -g -Wextra -pedantic -O1
SRCS = step8_macros.c types.c core.c reader.c linked_list.c printer.c hashmap.c env.c
OBJS = $(SRCS:.c=.o)

.PHONNY: all clean

all: step8_macros

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

step5_tco: $(OBJS)
	gcc $(CFLAGS) $(OBJS) -o $@ -lpcre -lgc

step6_file: $(OBJS)
	gcc $(CFLAGS) $(OBJS) -o $@ -lpcre -lgc

step7_quote: $(OBJS)
	gcc $(CFLAGS) $(OBJS) -o $@ -lpcre -lgc

step8_macros: $(OBJS)
	gcc $(CFLAGS) $(OBJS) -o $@ -lpcre -lgc

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@
