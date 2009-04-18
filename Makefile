all: test

reverse_sem.o: reverse_sem.h reverse_sem.c
	gcc -g -c reverse_sem.c $$(pkg-config --cflags glib-2.0)

scribl.o: scribl.h scribl.c
	gcc -g -c scribl.c $$(pkg-config --cflags glib-2.0)

test: test.c scribl.o reverse_sem.o
	gcc -g -o test test.c $$(pkg-config glib-2.0 --cflags --libs) $$(pkg-config --libs gthread-2.0) scribl.o reverse_sem.o

test_counter: test_counter.c scribl.o reverse_sem.o
	gcc -g -o test_counter test_counter.c $$(pkg-config glib-2.0 --cflags --libs) $$(pkg-config --libs gthread-2.0) scribl.o reverse_sem.o

clean:
	-rm -f *.o
	-rm -f test_counter

doc:
	doxygen doc/scribl.doxy

.PHONY: all clean doc
