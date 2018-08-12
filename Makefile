test_gtk_print : test_gtk_print.c 
	gcc -Wall -g $< -o $@ `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0` -lm -pg  
