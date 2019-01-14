TARGET	:= libengine.a
LIBLOGGING	:= libs/logging/liblogging.a
LIBUTILS	:= libs/utils/libutils.a
LIBNETUTILS	:= libs/netutils/libnetutils.a

CC		:= gcc
AR		:= ar
LIBS	:= 
LDFLAGS	:= -pthread

DEFINES	:= -DZALLOC_DEBUG
INCLUDE	:= -I../3rd-parties -I./include -I../libs
CFLAGS	:= -g -O0 -Wall -pthread $(DEFINES) $(INCLUDE)

SOURCE	:= $(wildcard ./src/*.c)
OBJS	:= $(patsubst %.c, %.o, $(SOURCE))

.PHONY : all objs clean rebuild

all : $(TARGET)

objs : $(OBJS)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

rebuild: clean all

clean :
	@rm -fr src/*.o
	@rm -fr $(TARGET)
	@make -C ../libs/logging clean
	@make -C ../libs/utils clean
	@make -C ../libs/netutils clean

$(TARGET) : $(OBJS) $(LIBLOGGING) $(LIBUTILS) $(LIBNETUTILS)
	$(AR) rsv $@ $(OBJS)

$(LIBLOGGING) :
	@make -C ../libs/logging

$(LIBUTILS) :
	@make -C ../libs/utils

$(LIBNETUTILS) :
	@make -C ../libs/netutils
