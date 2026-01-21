BIN	= calc
TESTER	= tester

CC	= gcc
CFLAGS	= -Werror -Wextra -Wall -O0 -g -Wno-override-init
LFLAGS	= -lreadline -lm
INC	= -Iinclude

SRCS	= $(addprefix src/, arena.c lexer.c parser.c evaluator.c swissmap.c)
#OBJS	= $(SRCS:.c=.o)

all: run

# Note: We want a full rebuild atm but if needed, move to an incremental one.
$(BIN): $(SRCS)
	$(CC) $(CFLAGS) $(INC) main.c $^ $(LFLAGS) -o $(BIN)

$(TESTER): $(SRCS)
	$(CC) -fsanitize=address $(CFLAGS) -DTESTER $(INC) tests.c $^ $(LFLAGS) -o $(TESTER)

clean:
	rm -rf $(BIN)

clean_tester:
	rm -rf $(TESTER)

fclean: clean clean_tester

re: clean $(BIN)

run: re
	./$(BIN)

tests: clean_tester $(TESTER)
	./$(TESTER)

docs_serve:
	cd mkdocs && mkdocs serve

docs_open:
	vim mkdocs/docs/documentation.md

docs_open_notes:
	vim mkdocs/docs/index.md

.PHONY: all clean re run tests clean_tester fclean docs_open docs_serve docs_open_notes
