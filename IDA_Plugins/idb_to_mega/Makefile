SRC=idb_to_mega.cpp
OBJS=idb_to_mega.o
CC=g++
LD=g++
CFLAGS=-D__IDP__ -D__PLUGIN__ -c -D__LINUX__ \
	   -I/usr/local/idaadv/sdk/include $(SRC)
LDFLAGS=--shared $(OBJS) -L/usr/local/idaadv -lida \
		--no-undefined -Wl,--version-script=./plugin.script
all:
	$(CC) $(CFLAGS)
	$(LD) $(LDFLAGS) -o idb_to_mega.plx
	cp idb_to_mega.plx /usr/local/idaadv/plugins

