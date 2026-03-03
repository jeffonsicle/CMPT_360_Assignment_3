CC:= gcc-13
CFLAGS := -std=c11 -Wall -Wextra -pedantic -O2 -g
LDFLAGS:=

#Valgrind
VALGRIND?=valgrind
VGFLAGS?= --leak-check=full --show-leak-kinds=all --track-origins=yes --errors-for-leak
kinds=all --error-exitcode=1
.PHONY: all clean check valgrind vg

all: vmsim

vmsim: vmsim.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

vmsim.o:vmsim.c
	$(CC) $(CFLAGS) -c $<

#Run sample input under Valgrind; fails build on leaks/errors
valgrind: vmsim
	$(VALGRIND) $(VGFLAGS) ./vmsim < tests/input.txt

vg: valgrind

clean:
	rm -f vmsim vmsim.o