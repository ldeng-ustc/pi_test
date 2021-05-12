CC=g++
CFLAGS=-std=c++17 -g -Wall -pthread -O2 -fPIC -I..
LDFLAGS= -lpthread -lrocksdb -lfmt -lxxhash

OBJECTS=pi.o sketch.o
EXEC=test
SHARED=libpidb.so

all: $(EXEC) $(SHARED)

.h.cc:

.cc.o:
	$(CC) -c $(CFLAGS) $< $(LDFLAGS) -o $@

$(EXEC): pi_test.cc $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(SHARED): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -shared -o $@

.PHONY: clean install uninstall
clean:
	rm -f $(wildcard *.o) $(EXEC) $(SHARED)

install: $(SHARED)
	sudo ln -s `realpath libpidb.so` /usr/local/lib
	sudo ln -s `realpath .`		/usr/local/include

uninstall:
	sudo rm /usr/local/lib/libpidb.so
	sudo rm /usr/local/include/pi
