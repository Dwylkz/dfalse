CC=gcc
CFLAGS += -std=gnu99

dfalse: dfalse.c

.PHONY: run
run:
	valgrind  --leak-check=full ./dfalse test.df

.PHONY: test
test:
	./dfalse test.df

.PHONY: clean
clean:
	rm *.o false
