#  -*- makefile -*-

#########################
# COMPILE CONFIGURATION #
#########################

# Credence
#TCLBASEDIR = /u/dherron/.local
#CFLAGS  = -I${TCLBASEDIR}/include
#LDFLAGS = -L${TCLBASEDIR}/lib -ldl -lsocket -lnsl -lc -lm -lgdbm -ltcl8.0

# PAIR.COM
#TCLBASEDIR = /usr/home/davidh/.local
#CFLAGS   = -I${TCLBASEDIR}/include -I/usr/local/include ${DEBUGFLAG} -DFREEBSD
#LDFLAGS  = -L${TCLBASEDIR}/lib  \
#     /usr/local/lib/libgdbm.a ${TCLBASEDIR}/lib/libtcl80.so.1.0 -lm


# LAPPY
TCLBASEDIR = /usr/home/david/.local
CFLAGS   = -I${TCLBASEDIR}/include -I/usr/local/include ${DEBUGFLAG} -DFREEBSD
LDFLAGS  = -L${TCLBASEDIR}/lib  \
      /usr/local/lib/libgdbm.a /usr/local/lib/libtcl80.so.1.0 -lm
BIN_INSTALL_DIR = ${TCLBASEDIR}/bin

DEBUGFLAG = -g

SNAPOBJS = tclAppInit.o scan.o tclgdbm.o memchan.o cgic.o grep.o

#######################
# COMPONENT LOCATIONS #
#######################

TCLGDBMDIR = ../Tclgdbm0.4
MEMCHANDIR = ../memchan1.3

###########
# TARGETS #
###########

snap: ${SNAPOBJS}
	$(CC) ${SNAPOBJS} ${LDFLAGS} ${DEBUGFLAG} -o snap

clean:
	rm -f ${SNAPOBJS} *.o snap a.out *~ test.db
	find .. -name '*~' -type f -print  | xargs rm

configure:
	cd $(MEMCHANDIR); ./configure
	cd $(TCLGDBMDIR); ./configure

install: snap
	cp snap ${BIN_INSTALL_DIR}


################
# DEPENDENCIES #
################

scan.o:       scan.c
tclAppInit.o: tclAppInit.c
cgic.o:       cgic.c cgic.h
grep.o:       grep.c

# These are generated from the component directories.
# There isn't a clean way of doing this, so here it is.

tclgdbm.o: $(TCLGDBMDIR)/tclgdbm.c 
	cd $(TCLGDBMDIR); $(CC) -c $(CFLAGS) -I. -DVERSION="\"0.4\"" \
		tclgdbm.c -o ../src/tclgdbm.o

memchan.o: $(MEMCHANDIR)/memchan.c
	cd $(MEMCHANDIR); $(CC) -c $(CFLAGS) -I. -DMEMCHAN_VERSION="\"1.3\"" \
		memchan.c -o ../src/memchan.o
