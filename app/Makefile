TARGET	:= app
LIBENGINE	:= ../engine/libengine.a
LIBLOGGING	:= libs/logging/liblogging.a
LIBUTILS	:= libs/utils/libutils.a
LIBNETUTILS	:= libs/netutils/libnetutils.a

CC		:= gcc
C++		:= g++
AR		:= ar
LIBS	:= 
LDFLAGS	:= -pthread 
LIBS	:= -L../engine -lengine -L../libs/logging -llogging -L../libs/utils -lutils -L../libs/netutils -lnetutils -luv
DEFINES	:= -DZALLOC_DEBUG
INCLUDE	:= -I../3rd-parties -Iinclude -I../libs -I../libs/utils -I../engine/include
CPPFLAGS	:= -std=c++11 -g -O0 -Wall -pthread $(DEFINES) $(INCLUDE)

SOURCE	:= $(wildcard src/*.cpp)
OBJS	:= $(patsubst %.cpp, %.o, $(SOURCE))

.PHONY : all objs clean rebuild

all : $(TARGET)

objs : $(OBJS)

%o : %cpp
	$(C++) $(CPPFLAGS) -c $<

rebuild: clean all

clean :
	@rm -fr src/*.o
	@rm -fr $(TARGET)
	@make -C ../engine clean
	@make -C ../libs/logging clean
	@make -C ../libs/utils clean
	@make -C ../libs/netutils clean

$(TARGET) : $(OBJS) $(LIBENGINE) $(LIBLOGGING) $(LIBUTILS) $(LIBNETUTILS)
	$(C++) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

$(LIBENGINE) :
	@make -C ../engine

$(LIBLOGGING) :
	@make -C ../libs/logging

$(LIBUTILS) :
	@make -C ../libs/utils

$(LIBNETUTILS) :
	@make -C ../libs/netutils
