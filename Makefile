include config.mk

.SUFFIXES: .c .o

all:	gzip

.o:
	${LD} ${LDFLAGS} -o $@ $< ${LDADD}

.c.o:
	${CC} ${CFLAGS} -c -o $@ $<

gzip: gzip.o miniz.o miniz_tdef.o miniz_tinfl.o
	${LD} ${LDFLAGS} -o $@ gzip.o miniz.o miniz_tdef.o miniz_tinfl.o ${LDADD}

gzip.o: gzip.c miniz.c miniz.h config.mk
	${CC} ${CFLAGS} -c -o $@ $<

install: all
	install -Dm 755 gzip ${DESTDIR}${BINDIR}

clean:
	rm -f gzip *.o ${LIB}
