VERS=4.6

all: fqz_comp

CC = g++
#CFLAGS  = -g -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
CFLAGS  = -O2 -g -fomit-frame-pointer -fstrict-aliasing -ffast-math -DNDEBUG -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -Wall -msse2

#CC=icpc
#CFLAGS  = -O3 -g -fomit-frame-pointer -fstrict-aliasing -fast -DNDEBUG -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -Wall -m64 -msse2

CFLAGS += -DPTHREADS
LIBS += -lpthread

#CFLAGS += -DORIG_MODEL

.c.o:
	$(CC) $(CFLAGS) -c $< 

fqz_comp: fqz_comp.o sfh.o
	$(CC) $(CFLAGS) -o $@ fqz_comp.o sfh.o $(LIBS)

clean:
	-rm *.o fqz_comp

dist:
	-rm -rf fqzcomp-$(VERS)
	mkdir fqzcomp-$(VERS)
	cp *.[ch] *.cdr fqzcomp-$(VERS)
	cp README Makefile fqzcomp-$(VERS)
	tar cvfz fqzcomp-$(VERS).tar.gz fqzcomp-$(VERS)
