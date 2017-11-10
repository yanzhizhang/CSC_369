SRCS = simpleloop.c matmul.c blocked.c
PROGS = simpleloop matmul blocked

all : $(PROGS)

$(PROGS) : % : %.c
	gcc -Wall -g -o $@ $<


traces: $(PROGS)
	./runit simpleloop
	./runit matmul 100
	./runit blocked 100 25

.PHONY: clean
clean : 
	rm -f simpleloop matmul blocked tr-*.ref *.marker *~
