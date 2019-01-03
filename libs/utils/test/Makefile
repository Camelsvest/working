# Makefile for test

export CROSS_COMPILE?=

C++	= $(CROSS_COMPILE)g++
CC 	= $(CROSS_COMPILE)gcc
AR 	= $(CROSS_COMPILE)ar
LD	= $(CROSS_COMPILE)ld
MAKE	= make
RANLIB  = $(CROSS_COMPILE)ranlib

PRJ_CFLAGS	+= ${CFLAGS}
PRJ_CFLAGS	+= -g -pthread -Wall -fno-inline -fstack-protector -fPIC

PRJ_LDFLAGS	+= -g -pthread -fPIC

export PRJ_ROOT	  := ${shell pwd}

vpath %.h ${PRJ_ROOT}
vpath %.h ${PRJ_ROOT}/..

PRJ_INCLUDES	+= -I.. -I../../../3rd-parties

PRJ_LIBS	+= -L.. -lutil

C_SRCS	:= ${shell find . -maxdepth 1 -name "*.c"}
C_OBJS	:= ${patsubst %.c, %.o, ${C_SRCS}}

TARGET		= hashtest
DEPS		= Makefile.deps

.PHONY: clean install uninstall

${TARGET}: ${C_OBJS} ../libutil.a
	$(CC) -o $@ ${C_OBJS} ${PRJ_LDFLAGS} ${PRJ_LIBS}

%.o: %.c ${DEPS}
	${CC} ${PRJ_CFLAGS} -o $@ -c $< ${PRJ_INCLUDES}

$(DEPS): $(C_SRCS)
	${CC} -MM ${PRJ_CFLAGS} $^ > $@ ${PRJ_INCLUDES}

-include $(DEPS)

../libz3util.a:
	make -C ../

clean:
	@rm ${TARGET}
	@rm ${DEPS}
	@rm ${C_OBJS}
