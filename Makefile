all: test_counter


scribl.o: scribl.h scribl.c
	gcc -g -c scribl.c $$(pkg-config --cflags glib-2.0)

test_counter: test_counter.c scribl.o
	gcc -g -o test_counter test_counter.c $$(pkg-config glib-2.0 --cflags --libs) $$(pkg-config --libs gthread-2.0) scribl.o

clean:
	-rm -f *.o
	-rm -f test_counter

doc:
	make -C doc

.PHONY: all clean doc
