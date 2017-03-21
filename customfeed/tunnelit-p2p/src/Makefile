CC = gcc
CFLAGS = -std=gnu99 -Wall -DNDEBUG=1 -UDEBUG -g -O2
CPPFLAGS =  -DHAVE_CONFIG_H
INSTALL = /usr/bin/install -c

PREFIX = /usr/local
EXEC_PREFIX = ${prefix}
prefix = $(PREFIX)
exec_prefix = $(EXEC_PREFIX)
bindir = ${exec_prefix}/bin
datarootdir = ${prefix}/share
mandir = ${datarootdir}/man

BINS = tunnelit-p2p


tunnelit-p2p: tunnelit-p2p.o ethertun.o net.o

tunnelit-p2p.o: tunnelit-p2p.c ethertun.h tunnelit-helper.h tunnelit-common.h net.h config.h

ethertun.o: ethertun.c ethertun.h

net.c: net.h config.h

clean:
	rm -f *.o $(BINS)
