CC=gcc
PEDANTIC_PARANOID_FREAK =       -g -O0 -Wall -Wshadow -Wcast-align \
				-Waggregate-return -Wstrict-prototypes \
				-Wredundant-decls -Wnested-externs \
				-Wpointer-arith -Wwrite-strings -finline-functions -Werror
REASONABLY_CAREFUL_DUDE =	-Wall 
NO_PRAYER_FOR_THE_WICKED =	-w
WARNINGS = 			$(PEDANTIC_PARANOID_FREAK)

CFLAGS = -g -O2 $(WARNINGS)
LDFLAGS= 

TEST_FILES = $(wildcard *.in)
TEST_LIST = $(patsubst %.in,%.test,$(TEST_FILES))
TEST_LIST += test_leak.test test_leak2.test
C_FILES = $(wildcard *.c)
BIN_FILES = $(patsubst %.c,%,$(C_FILES))
O_FILES = $(patsubst %.c,%.o,$(C_FILES))


mem_alloc_test:mem_alloc.c
	$(CC) -DMAIN -DMEMORY_SIZE=2048 $(CFLAGS) $(LDFLAGS) $< -o $@

mem_shell:mem_shell.c mem_alloc.c
	$(CC) -DMEMORY_SIZE=512 $(CFLAGS) $(LDFLAGS) $^ -o $@

test_end_checking:test_end_checking.c mem_alloc.c
	$(CC) -DMEMORY_SIZE=2048 -D__CHECK_END__ $(CFLAGS) $(LDFLAGS) $^ -o $@

test_free_checking:test_free_checking.c mem_alloc.c
	$(CC) -DMEMORY_SIZE=2048 $(CFLAGS) $(LDFLAGS) $^ -o $@

libmalloc.o:mem_alloc.c
#	$(CC) -DMEMORY_SIZE=8388608 $(CFLAGS) -fPIC -c $< -o $@
	$(CC) -DMEMORY_SIZE=65536 $(CFLAGS) -fPIC -c $< -o $@

libmalloc.so:libmalloc.o
	$(CC) -shared -Wl,-soname,$@ $< -o $@

test_ls:libmalloc.so
	LD_PRELOAD=./libmalloc.so ls
	LD_PRELOAD=""

leak_test: leak_test.c libmalloc.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

%.out: %.in ./mem_shell
	@cat $< | ./mem_shell >/dev/null 2>$@

%.test: %.out %.out.expected
	@if diff $^  >/dev/null; then \
	  echo "**** Test $@ Passed *****"; \
	else \
	  echo "**** Test $@ FAILED *****"; \
	  diff -y $^ ;\
	fi

test_leak.out: leak_test ./libmalloc.so
	@LD_PRELOAD=./libmalloc.so ./$< > /dev/null 2>$@
	@LD_PRELOAD=""

test_leak2.out: leak_test ./libmalloc.so
	@LD_PRELOAD=./libmalloc.so ./$< do_not_leak > /dev/null 2>$@
	@LD_PRELOAD=""

test: $(TEST_LIST)

clean:
	rm -f $(BIN_FILES) *.o *~ *.so mem_alloc_test

.PHONY: clean test
