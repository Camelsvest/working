###################################################
# Public Makefile Template for compiling a library
# 1. https://www.cnblogs.com/sysu-blackbear/p/4034394.html
# 2. https://blog.csdn.net/sunrier/article/details/7664734
# 3. https://blog.csdn.net/lzx_bupt/article/details/7988192
# 4. http://www.cnblogs.com/prettyshuang/p/5552328.html
###################################################
  
#target you can change test to what you want
#.......lib*.so
TARGET  := libutils.a

#compile and lib parameter
#....
CC      := gcc
AR	:= ar
LIBS    :=
LDFLAGS := -pthread

DEFINES := -DZALLOC_DEBUG 
INCLUDE := -I../../3rd-parties/ -I../
CFLAGS  := -g -O0 -Wall -Wno-pointer-to-int-cast -pthread $(DEFINES) $(INCLUDE)
CXXFLAGS:= $(CFLAGS) -DHAVE_CONFIG_H
  
#i think you should do anything here
#...............
  
#source file
#..........c..cpp.............o..
SOURCE  := $(wildcard *.c)
OBJS    := $(patsubst %.c, %.o, $(SOURCE))
  
.PHONY : all objs clean rebuild
  
all : $(TARGET) $(TEST)
  
objs : $(OBJS)

%.o : %.c
	$(CC) $(CFLAGS) -c $<
  
rebuild: clean all
                
clean :
	@rm -fr *.o
	@rm -fr $(TARGET) $(TEST)
  
$(TARGET) : $(OBJS)
	$(AR) rsv $@ $(OBJS)
