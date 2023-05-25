###########################################
# Simple Makefile for HIDAPI test program
#
# Alan Ott
# Signal 11 Software
# 2010-06-01
###########################################

all: hidapi.dll hidtest.exe writetest.exe

CC=gcc
COBJS=hidapi.o
OBJS=$(COBJS) $(CPPOBJS)
CFLAGS= -O2 -c -m32 -std=gnu11
LIBS= -lsetupapi -m32
DLL_LDFLAGS = -mwindows -lsetupapi
DLLNAME=hidapi.dll
LINKLIB=lib$(DLLNAME).a



hidapi.dll: $(OBJS)
	$(CC) -g $^ $(DLL_LDFLAGS) -o $(DLLNAME) -shared -Wl,--dll -Wl,--out-implib=$(LINKLIB) -m32

hidtest.exe: hidtest.o
	gcc hidtest.c -I../hidapi $(LINKLIB) -m32 -o hidtest.exe

writetest.exe: writetest.o ../xpre.o ../jparse.o ../tiny-json/tiny-json.o
	gcc writetest.c -I../hidapi ./x32/$(LINKLIB) ../xpre.o ../jparse.o ../tiny-json/tiny-json.o -m32 -o writetest.exe
	
$(COBJS): %.o: %.c
	$(CC) $(CFLAGS) $< -o $@

$(CPPOBJS): %.o: %.cpp
	$(CXX) $(CFLAGS) $< -o $@

clean:
	del *.o hidtest.exe $(LINKLIB) $(DLLNAME) *.bak

.PHONY: clean
