.POSIX:

NAME = stamd

LIBGIT_INC = -I/usr/local/include
LIBGIT_LIB = -L/usr/local/lib -lgit2

# use system flags.
STAMD_CFLAGS = ${LIBGIT_INC} ${CFLAGS}
STAMD_LDFLAGS = ${LIBGIT_LIB} ${LDFLAGS}
STAMD_CPPFLAGS = -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE -D_BSD_SOURCE

SRC = \
	stamd.c\
	stamd-index.c
COMPATSRC = \
	reallocarray.c\
	strlcat.c\
	strlcpy.c
BIN = \
	stamd\
	stamd-index
HDR = 

COMPATOBJ = \
	article.o\
	config.o\
	line.o\
	queue.o\
	text.o\

OBJ = ${SRC:.c=.o} ${COMPATOBJ}

all: ${BIN}

.o:
	${CC} -o $@ ${LDFLAGS}

.c.o:
	${CC} -o $@ -c $< ${STAMD_CFLAGS} ${STAMD_CPPFLAGS}

${OBJ}: ${HDR}

stamd: stamd.o ${COMPATOBJ}
	${CC} -o $@ stamd.o ${COMPATOBJ} ${stamd_LDFLAGS}

stamd-index: stamd-index.o ${COMPATOBJ}
	${CC} -o $@ stamd-index.o ${COMPATOBJ} ${stamd_LDFLAGS}

clean:
	rm -f ${BIN} ${OBJ}

.PHONY: all clean

