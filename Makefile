TARGET	:= libengine.a

CC		:= gcc
AR		:= ar
LIBS	:= 
LDFLAGS	:= -pthread

DEFINES	:=
INCLUDE	:= -I./3rd-parties -I./include
CFLAGS	:= -g -Wall -pthread $(DEFINES) $(INCLUDE)

SOURCE	:= $(wildcard ./src/*.c)
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

$(TARGET) : $(OBJS)
	$(AR) rsv $@ $(OBJS)

