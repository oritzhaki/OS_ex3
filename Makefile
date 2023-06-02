ex3.out: *.c *.h
	gcc -pthread -w -g *.c -o $@