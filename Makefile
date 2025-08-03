CC ?= cc
CFLAGS ?= -ggdb3 -O2


INC = `pkg-config --cflags zlib fuse3`
LIBS = `pkg-config --libs zlib fuse3`

rpafs: decompressor.o unpickle.o rpafs.o fs.o fuse.o
	$(CC) $^ $(INC) $(LIBS) $(CFLAGS) $(LDFLAGS) -o $@

rpafs.o: rpafs.c
	$(CC) -c $< $(INC) $(CFLAGS) -o $@

unpickle.o: unpickle.c
	$(CC) -c $< $(INC) $(CFLAGS) -o $@

decompressor.o: decompressor.c
	$(CC) -c $< $(INC) $(CFLAGS) -o $@

fs.o: fs.c
	$(CC) -c $< $(INC) $(CFLAGS) -o $@

fuse.o: fuse.c
	$(CC) -c $< $(INC) $(CFLAGS) -o $@

clean:
	rm -rf rpafs *.o

.PHONY: clean
