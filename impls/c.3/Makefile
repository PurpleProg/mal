SRCS = step1_read_print.c reader.c linked_list.c


clean:
	rm step1_read_print


step0_repl: step0_repl.c
	gcc -Wall step0_repl.c -o step0_repl


step1_read_print: $(SRC)
	gcc -Wall $(SRCS) -o step1_read_print -lpcre -lgc
