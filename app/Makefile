TARGET	:= app
LIBENGINE	:= ../engine/libengine.a
LIBLOGGING	:= libs/logging/liblogging.a
LIBUTILS	:= libs/utils/libutils.a
LIBNETUTILS	:= libs/netutils/libnetutils.a

CC		:= gcc
AR		:= ar
LIBS	:= 
LDFLAGS	:= -pthread 
LIBS	:= -L../engine -lengine -L../libs/utils -lutils -L../libs/netutils -lnetutils -L../libs/logging -llogging
DEFINES	:= -DZALLOC_DEBUG
INCLUDE	:= -I../3rd-parties -Iinclude -I../libs -I../engine/include
CFLAGS	:= -g -O0 -Wall -pthread $(DEFINES) $(INCLUDE)

SOURCE	:= $(wildcard src/*.c)
OBJS	:= $(patsubst %.c, %.o, $(SOURCE))

.PHONY : all objs clean rebuild

all : $(TARGET)

objs : $(OBJS)

%o : %c
	$(CC) $(CFLAGS) -c $<

rebuild: clean all

clean :
	@rm -fr src/*.o
	@rm -fr $(TARGET)
	@make -C ../engine clean
	@make -C ../libs/logging clean
	@make -C ../libs/utils clean
	@make -C ../libs/netutils clean

$(TARGET) : $(OBJS) $(LIBENGINE) $(LIBLOGGING) $(LIBUTILS) $(LIBNETUTILS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

$(LIBENGINE) :
	@make -C ../engine

$(LIBLOGGING) :
	@make -C ../libs/logging

$(LIBUTILS) :
	@make -C ../libs/utils

$(LIBNETUTILS) :
	@make -C ../libs/netutils
