

ifdef  x64

BUILDARCH=64
RESTARGET= 

else

BUILDARCH=32
RESTARGET= --target=pe-i386

endif





CC = gcc
CFLAGS = -std=gnu11 -Wall -O2 -m$(BUILDARCH) -Wno-aggressive-loop-optimizations
LIBS= -lsetupapi -m$(BUILDARCH) -s


src = $(wildcard *.c)
src += $(wildcard tiny-json/*.c)
obj = $(src:.c=.o)
# dep = $(obj:.o=.d) 

.PHONY: build all clean

build: sa-c4.exe

all: clean build

clean::
	rm -rf $(dep)
	rm -rf $(obj)
	rm -rf *.exe


sa-c4.exe: sa-c4.o jas.o tiny-json/tiny-json.o curljson.o xpre.o libas.o help.o ctrl_c4.o osbf.o
	windres --output-format=coff $(RESTARGET) -i sa-c4.rc -o res.o
	$(CC) $(CFLAGS) -s -o $@ $^ curl/x$(BUILDARCH)/libcurl.dll.a hidapi/x$(BUILDARCH)/libhidapi.dll.a res.o

	
xpre.exe: xpre.o jparse.o tiny-json/tiny-json.o 
	gcc $(CFLAGS) -o $@ $^ 
	
jparse.exe: jparse.o tiny-json/tiny-json.o
	gcc $(CFLAGS) -o $@ $^
	
getjson.exe: getjson.o
	gcc $(CFLAGS) -o $@ $^ curl/libcurl.dll.a


# -include $(dep)

%.d: %.c
	$(CC) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

