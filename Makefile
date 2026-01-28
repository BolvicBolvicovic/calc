BIN	= calc
VERSION = v0.1
TESTER	= tester

CC	= gcc
CFLAGS	= -Werror -Wextra -Wall -O0 -g -Wno-override-init -march=native
BFLAGS	= -O3 -Wno-override-init -march=native -fno-pie -no-pie
LFLAGS	= -lreadline -lm
INC	= -Iinclude

SRCS	= $(addprefix src/, arena.c lexer.c parser.c evaluator.c swissmap.c $(addprefix builtins/, polynomials.c roots.c trigo.c electricity.c))
#OBJS	= $(SRCS:.c=.o)

all: run

build: $(SRCS)
	$(CC) $(BFLAGS) $(INC) main.c $^ $(LFLAGS) -o $(BIN)-$(VERSION)

# Note: We want a full rebuild atm but if needed, move to an incremental one.
$(BIN): $(SRCS)
	$(CC) $(CFLAGS) $(INC) main.c $^ $(LFLAGS) -o $(BIN)

$(TESTER): $(SRCS)
	$(CC) -fsanitize=address -DTESTER $(CFLAGS) $(INC) tests.c $^ $(LFLAGS) -o $(TESTER)

$(TESTER)-perf: $(SRCS)
	$(CC) -g -DTESTER $(BFLAGS) $(INC) tests.c $^ $(LFLAGS) -o $(TESTER)-perf

clean:
	rm -rf $(BIN)

clean_testers:
	rm -rf $(TESTER) $(TESTER)-perf

fclean: clean clean_testers

re: clean $(BIN)

run: re
	./$(BIN)

tests: clean_testers $(TESTER) $(TESTER)-perf
	./$(TESTER) && ./$(TESTER)-perf

docs_serve:
	cd mkdocs && mkdocs serve

docs_open:
	vim mkdocs/docs/documentation.md

docs_open_notes:
	vim mkdocs/docs/index.md

.PHONY: all clean re run tests clean_tester fclean docs_open docs_serve docs_open_notes
