INTERPRETER	= calc
VM		= calc-vm
VERSION		= v0.1
TESTER		= tester
BENCH		= benchmark

CC		= gcc
CFLAGS		= -Werror -Wextra -Wall -O0 -g -Wno-override-init -march=native
BFLAGS		= -O3 -Wno-override-init -march=native -fno-pie -no-pie
LFLAGS		= -lreadline -lm
INC		= -Iinclude

SRCS_INT	= $(addprefix src/interpreter/, lexer.c parser.c evaluator.c errors.c $(addprefix builtins/, polynomials.c roots.c trigo.c electricity.c plot.c))
SRCS_VM 	= $(addprefix src/vm/, chunk.c value.c vm.c compiler.c scanner.c context.c)
SRCS_ANY	= $(addprefix src/, arena.c swissmap.c)

all: run

build_int: $(SRCS_INT) $(SRCS_ANY)
	$(CC) $(BFLAGS) -DINTERPRETER_BUILD $(INC) main.c $^ $(LFLAGS) -o $(INTERPRETER)-$(VERSION)

build_vm: $(SRCS_VM) $(SRCS_ANY)
	$(CC) $(BFLAGS) $(INC) main.c $^ $(LFLAGS) -o $(VM)-$(VERSION)

# Note: We want a full rebuild atm but if needed, move to an incremental one.
$(INTERPRETER): $(SRCS_INT) $(SRCS_ANY)
	$(CC) $(CFLAGS) -DINTERPRETER_BUILD $(INC) main.c $^ $(LFLAGS) -o $@

$(VM): $(SRCS_VM) $(SRCS_ANY)
	$(CC) $(CFLAGS) $(INC) main.c $^ $(LFLAGS) -o $@

$(TESTER): $(SRCS_INT) $(SRCS_VM) $(SRCS_ANY)
	$(CC) -fsanitize=address -DTESTER $(CFLAGS) $(INC) tests.c $^ $(LFLAGS) -lpthread -o $@

$(BENCH): $(SRCS_INT) $(SRCS_ANY)
	$(CC) -g $(BFLAGS) $(INC) benchmark.c $^ $(LFLAGS) -o $@

clean:
	rm -rf $(INTERPRETER) $(VM)
	rm -rf $(TESTER) $(BENCH)

vm: clean $(VM)
	./$(VM) $(ARGS)

re: clean $(INTERPRETER)

run: re
	./$(INTERPRETER)

tests: clean $(TESTER)
	./$(TESTER)

bench: clean $(BENCH)
	perf stat ./$(BENCH) > /dev/null

serve_docs:
	cd mkdocs && mkdocs serve

open_docs:
	vim mkdocs/docs/documentation.md

open_notes:
	vim mkdocs/docs/index.md

.PHONY: all clean re run tests bench serve_docs open_docs open_notes build_int build_vm
