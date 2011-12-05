SRC=sel.cpp
OBJS=sel.o
CC=g++
LD=g++
CFLAGS=-D__IDP__ -D__PLUGIN__ -c -D__LINUX__ \
	   -I/usr/local/idaadv/sdk/include $(SRC)
LDFLAGS=--shared $(OBJS) -L/usr/local/idaadv -lida \
		--no-undefined -Wl,--version-script=./plugin.script
all:
	$(CC) $(CFLAGS)
	$(LD) $(LDFLAGS) -o sel.plx
	cp sel.plx /usr/local/idaadv/plugins

