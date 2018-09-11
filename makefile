# makefile for Penguin derived from makefile for Otter2.0 and Otter2.2

.SUFFIXES: .pcn .pam

LIBRARIES = -lc 
WINLIBS = -lsuntool -lsunwindow -lpixrect

#  DFLAGS controls conditional compilation:
#    -DTP_UNIX specifies basic UNIX compilation.
#    -DTP_SUN does something cute, but it seems to work only on Suns.
#    -DTP_NO_STDLIB is necessary on some UNIXes, because stdlib.h is absent.
#    (-DTURBO_C is for PC.)
#    (-DTHINK_C is for Macintosh.)
#    (-DROO is for ROO, the shared-memory parallel version.)

DFLAGS = -DTP_UNIX

#GNU C compiler: gcc generates faster code on Sun3, cc better on Sun4.
#CC = gcc

# optimized
CFLAGS = -O $(DFLAGS)

# save symbols for dbx
# CFLAGS = -g $(DFLAGS)

# gprof profiling
#CFLAGS = -pg -O $(DFLAGS)

PENGUINS = 1

FILES =   av.c io.c share.c fpa.c clocks.c unify.c demod.c weight.c imd.c is.c clause.c options.c resolve.c index.c paramod.c formula.c process.c misc.c lrpo.c interface.c

OBJECTS = av.o io.o share.o fpa.o clocks.o unify.o demod.o weight.o imd.o is.o clause.o options.o resolve.o index.o paramod.o formula.o process.o misc.o lrpo.o interface.o

.c.o: ; cc -c $(CFLAGS) $*.c $*.o

.pcn.pam: ; pcncomp -c $*.pcn

m_penguin: $(OBJECTS) m_penguin.pam
	pcncomp -v -n $(PENGUINS) m_penguin.pam $(OBJECTS) -mm m_penguin -o m_penguin 

# m_penguin with debugger
m_penguind: $(OBJECTS) m_penguin.pam
	pcncomp -pdb -n $(PENGUINS) m_penguin.pam $(OBJECTS) -mm m_penguin -o m_penguind

lint:
	lint $(DFLAGS) $(FILES)

proto:
	csh make_proto $(FILES)

allfunctions:
	csh make_allfunctions $(FILES)

clean:
	/bin/rm *.o

extags: 
	ctags main.c $(FILES)

m_penguin.pam: pheader.h
b_penguin.pam: pheader.h
$(OBJECTS): header.h
$(OBJECTS): types.h
$(OBJECTS): macros.h
# $(OBJECTS): cos.h
# $(OBJECTS): proto.h
