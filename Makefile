DEV_FLAGS=-g -pedantic -Wall

all: test

reverse_sem.o: reverse_sem.h reverse_sem.c
	gcc $(DEV_FLAGS) -c reverse_sem.c $$(pkg-config --cflags glib-2.0)

scribl.o: scribl.h scribl.c logging.h reverse_sem.h
	gcc $(DEV_FLAGS) -c scribl.c $$(pkg-config --cflags glib-2.0) -DG_LOG_DOMAIN=\"scribl\"

logging.o: logging.h logging.c
	gcc $(DEV_FLAGS) -c logging.c $$(pkg-config --cflags glib-2.0)

test: test.c scribl.o reverse_sem.o logging.o
	gcc $(DEV_FLAGS) -o test test.c -lm $$(pkg-config glib-2.0 --cflags --libs) $$(pkg-config --libs gthread-2.0) scribl.o reverse_sem.o logging.o

clean:
	-rm -f *.o
	-rm -f test
	-rm -f scribl-*.log

doc:
	doxygen doc/Doxyfile

.PHONY: all clean doc
