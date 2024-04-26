PROG = GitInsights
LIB = GitInsights.a
XLIB = /usr/local/lib64/shslib.a  -lcurl
DEF = /usr/local/include
PRG = /usr/local/bin/$(PROG)

FILES = \
	$(LIB)(GitInsights.o)

.SILENT:

$(PRG): $(LIB) $(XLIB)
	echo "using gcc to load $(PRG)"
	gcc -o $(PRG) $(LIB) $(XLIB)

$(LIB): $(FILES)

$(FILES): $(DEF)/shslib.h

clean:
	rm -f $(LIB)

all:
	make clean
	make

.PRECIOUS: $(LIB)

