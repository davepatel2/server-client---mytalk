all: mytalk

mytalk: mytalk.c
	gcc -c -g -Wall -I ~pn-cs357/Given/Talk/include mytalk.c -o mytalk.o
	gcc -g -Wall -L ~pn-cs357/Given/Talk/lib64 -o mytalk mytalk.o -ltalk -lncurses
	
	
clean:
	rm *.o all