#
# Makefile that builds btest and other helper programs for the CS:APP data lab
# 
CC = gcc
CFLAGS = -O1 -Wall -m32
LIBS = -lm
GITFLAGS = -q --no-verify --allow-empty

all: btest fshow ishow submit commit

btest: btest.c bits.c decl.c tests.c btest.h bits.h
	$(CC) $(CFLAGS) $(LIBS) -o btest bits.c btest.c decl.c tests.c

fshow: fshow.c
	$(CC) $(CFLAGS) -o fshow fshow.c

ishow: ishow.c
	$(CC) $(CFLAGS) -o ishow ishow.c

# Forces a recompile. Used by the driver program. 
btestexplicit:
	$(CC) $(CFLAGS) $(LIBS) -o btest bits.c btest.c decl.c tests.c 

clean:
	rm -f *.o btest fshow ishow lab1-handin.zip *~

commit:
	-@git add . -A --ignore-errors
	-@while (test -e .git/index.lock); do sleep 0.1; done
	-@(id -un && uname -a && uptime && (head -c 20 /dev/urandom | hexdump -v -e '"%02x"')) | git commit -F - $(GITFLAGS)

submit:
	rm -rf lab1-handin.zip
	zip lab1-handin.zip bits.c
