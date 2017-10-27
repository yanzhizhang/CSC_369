all: traffic

traffic: traffic.o cars.o
	gcc -Wall -g -pthread -o $@ $^

%.o : %.c traffic.h
	gcc -Wall -g -c $<

clean : 
	rm -f *.o traffic *~

