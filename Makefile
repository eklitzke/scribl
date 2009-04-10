test_counter: test_counter.c counter.c
	gcc -o test_counter test_counter.c $(pkg-config glib-2.0 --cflags --libs)
