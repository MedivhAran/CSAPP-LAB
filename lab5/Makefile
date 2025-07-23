#
# Student makefile for Cache Lab
# Note: requires a 64-bit x86-64 system 
#
CC = gcc
CFLAGS = -g -Wall -Werror -std=c99 -m64
GITFLAGS = -q --no-verify --allow-empty

all: csim test-trans tracegen commit
	# Generate a handin tar file each time you compile
	-rm lab5-handin.zip
	-zip lab5-handin.zip  csim.c trans.c 

csim: csim.c cachelab.c cachelab.h
	$(CC) $(CFLAGS) -o csim csim.c cachelab.c -lm 

test-trans: test-trans.c trans.o cachelab.c cachelab.h
	$(CC) $(CFLAGS) -o test-trans test-trans.c cachelab.c trans.o 

tracegen: tracegen.c trans.o cachelab.c
	$(CC) $(CFLAGS) -O0 -o tracegen tracegen.c trans.o cachelab.c

trans.o: trans.c
	$(CC) $(CFLAGS) -O0 -c trans.c

commit:
	-@git add . -A --ignore-errors
	-@while (test -e .git/index.lock); do sleep 0.1; done
	-@(id -un && uname -a && uptime && (head -c 20 /dev/urandom | hexdump -v -e '"%02x"')) | git commit -F - $(GITFLAGS)

#
# Clean the src dirctory
#
clean:
	rm -rf *.o
	rm -f *.tar
	rm -f csim
	rm -f test-trans tracegen
	rm -f trace.all trace.f*
	rm -f .csim_results .marker
	rm lab5-handin.zip
