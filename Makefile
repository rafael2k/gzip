include config.mk

LIB_UTIL = eprint readn
LIB = ${LIB_UTIL:S/$/.o/:S/^/util\//}

.SUFFIXES: .c .o

all:	gzip

.o:
	${LD} ${LDFLAGS} -o $@ $< ${LDADD}

.c.o:
	${CC} ${CFLAGS} -c -o $@ $<

gzip: gzip.o miniz.o util.a
	${LD} ${LDFLAGS} -o $@ gzip.o miniz.o util.a ${LDADD}

gzip.o: gzip.c util.h miniz.c miniz.h config.mk
	${CC} ${CFLAGS} -c -o $@ $<

util.a:	${LIB}
	${AR} -r -c $@ ${LIB}
	ranlib $@

install: all
	install -Dm 755 gzip ${DESTDIR}${BINDIR}

clean:
	rm -f gzip *.o ${LIB} util.a
