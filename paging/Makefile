all: a.out see.R
a.out: t4.o a4.o 
	gcc -g t4.o a4.o
t4.o: t4.c t4.h 
	gcc -g -c t4.c 
a4.o: a4.c t4.h 
	gcc -g -c a4.c 
t4.c: /comp/111/a/a4/t4.c
	ln -s /comp/111/a/a4/t4.c .
see.R: /comp/111/a/a4/see.R
	ln -s /comp/111/a/a4/see.R .
t4.h: /comp/111/a/a4/t4.h 
	ln -s /comp/111/a/a4/t4.h .
