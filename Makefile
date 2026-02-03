BIN	= calc
VERSION = v0.1
TESTER	= tester
BENCH	= benchmark

CC	= gcc
CFLAGS	= -Werror -Wextra -Wall -O0 -g -Wno-override-init -march=native
BFLAGS	= -O3 -Wno-override-init -march=native -fno-pie -no-pie
LFLAGS	= -lreadline -lm
INC	= -Iinclude

SRCS	= $(addprefix src/, arena.c lexer.c parser.c evaluator.c swissmap.c errors.c $(addprefix builtins/, polynomials.c roots.c trigo.c electricity.c plot.c))

all: run

build: $(SRCS)
	$(CC) $(BFLAGS) $(INC) main.c $^ $(LFLAGS) -o $(BIN)-$(VERSION)

# Note: We want a full rebuild atm but if needed, move to an incremental one.
$(BIN): $(SRCS)
	$(CC) $(CFLAGS) $(INC) main.c $^ $(LFLAGS) -o $@

$(TESTER): $(SRCS)
	$(CC) -fsanitize=address -DTESTER $(CFLAGS) $(INC) tests.c $^ $(LFLAGS) -o $@

$(BENCH): $(SRCS)
	$(CC) -g $(BFLAGS) $(INC) benchmark.c $^ $(LFLAGS) -o $@

clean:
	rm -rf $(BIN)
	rm -rf $(TESTER) $(BENCH)

re: clean $(BIN)

run: re
	./$(BIN)

tests: clean $(TESTER)
	./$(TESTER)

bench: clean $(BENCH)
	./$(BENCH)

serve_docs:
	cd mkdocs && mkdocs serve

open_docs:
	vim mkdocs/docs/documentation.md

open_notes:
	vim mkdocs/docs/index.md

.PHONY: all clean re run tests bench serve_docs open_docs open_notes
